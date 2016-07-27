#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/log.h>
#include <putki/builder/resource.h>
#include <putki/builder/build-db.h>
#include <putki/builder/db.h>

#include "kosmos-builder-utils/pngutil.h"
#include "kosmos-builder-utils/jpge.h"
#include "kosmos-builder-utils/textureconfig.h"

#include <inki/types/kosmos/texture.h>

#include <iostream>

namespace {
	const char *builder_version = "texture-builder-3";
}

struct texbuilder : putki::builder::handler_i
{
	static void resize(inki::TextureResizeMode mode, int width, int height, int *out_width, int *out_height)
	{
		if (mode == inki::RESIZE_NONE)
		{
			*out_width = width;
			*out_height = height;
			return;
		}

		*out_width = 1;
		*out_height = 1;

		while (*out_width < width) *out_width *= 2;
		while (*out_height < height) *out_height *=2;

		if (mode == inki::RESIZE_UNCROP_POW2SQUARE)
		{
			if (*out_width > *out_height)
			{
				*out_height = *out_width;
			}
			else if (*out_height > *out_width)
			{
				*out_width = *out_height;
			}
		}
	}

	virtual const char *version() {
		return builder_version;
	}

	virtual bool handle(putki::builder::build_context *context, putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj)
	{
		inki::texture *texture = (inki::texture *) obj;

		// this is used for atlas lookups later.
		texture->id = path;

		// this object supplies its own defaults on initialisation
		inki::texture_configuration config;
		if (texture->configuration)
		{
			putki::build_db::add_input_dependency(record, putki::db::pathof(input, texture->configuration));
			config = *texture->configuration;
		}

		putki::build_db::add_external_resource_dependency(record, texture->source.c_str(), putki::resource::signature(builder, texture->source.c_str()).c_str());

		// First load the information for the texture and fill in width & height
		ccgui::pngutil::loaded_png pnginfo;
		if (ccgui::pngutil::load_info(putki::resource::real_path(builder, texture->source.c_str()).c_str(), &pnginfo))
		{
			texture->width = texture->source_width = pnginfo.width;
			texture->height = texture->source_height = pnginfo.height;
			ccgui::pngutil::free(&pnginfo);
		}
		else
		{
			RECORD_WARNING(record, "Could not load source file!");
			return false;
		}

		inki::texture_output_format *outputFormat = config.output_format(putki::builder::config(builder));

		// If no output is needed
		if (!outputFormat)
		{
			RECORD_INFO(record, "No output format, skipping generation")
			return false;
		}


		int out_width, out_height;
		resize(outputFormat->resize_mode, pnginfo.width, pnginfo.height, &out_width, &out_height);

		texture->width = out_width;
		texture->height = out_height;

		// note: assume uncrop
		const float u0 = 0;
		const float v0 = 0;
		const float u1 = float(pnginfo.width) / float(out_width);
		const float v1 = float(pnginfo.height) / float(out_height);

		// load
		ccgui::pngutil::loaded_png png;
		if (!ccgui::pngutil::load(putki::resource::real_path(builder, texture->source.c_str()).c_str(), &png))
		{
			RECORD_WARNING(record, "Failed to load source file!");
			return false;
		}
		
		if (outputFormat->premultiply_alpha)
		{
			for (size_t i=0;i<png.width*png.height;i++)
			{
				unsigned char *ptr = (unsigned char*)&png.pixels[i];
				ptr[0] = ptr[0] * ptr[3] / 255;
				ptr[1] = ptr[1] * ptr[3] / 255;
				ptr[2] = ptr[2] * ptr[3] / 255;
			}
		}

		// uncrop
		unsigned int *outData = png.pixels;
		if (out_width != png.width || out_height != png.height)
		{
			// only do uncrop
			RECORD_INFO(record, "Uncropping to " << out_width << "x" << out_height)
			outData = new unsigned int[out_width * out_height];

			for (int y=0;y<out_height;y++)
			{
				int srcy = y < png.height ? y : png.height - 1;
				unsigned int *srcLine = png.pixels + png.width * srcy;
				unsigned int *dstLine = outData + out_width * y;

				for (int x=0;x<png.width;x++)
					*dstLine++ = *srcLine++;

				// fill the rest with the last pixel
				for (int x=png.width;x<out_width;x++)
					*dstLine++ = *(srcLine-1);
			}
		}

		std::string path_res(path);
		path_res.append("_out");
		std::string path_res_data(path_res + "_data");

		if (outputFormat->rtti_type_ref() == inki::texture_output_format_raw::type_id())
		{
			inki::texture_output_raw *raw_tex = inki::texture_output_raw::alloc();
			
			texture->output = raw_tex;
			texture->output->data = inki::data_container::alloc();
			texture->output->data->config = outputFormat->storage_configuration;
			std::vector<unsigned char> & bytesOut = texture->output->data->bytes;

			// RGBA
			for (int i=0;i<out_width * png.height;i++)
			{
				// RGBA
				bytesOut.push_back((outData[i] >> 16) & 0xff);
				bytesOut.push_back((outData[i] >>  8) & 0xff);
				bytesOut.push_back((outData[i] >>  0) & 0xff);
				bytesOut.push_back((outData[i] >> 24) & 0xff);
			}
			
			add_output(context, record, path_res.c_str(), raw_tex);
			add_output(context, record, path_res_data.c_str(), raw_tex->data);
		}
		else if (outputFormat->rtti_type_ref() == inki::texture_output_format_png::type_id())
		{
			// these are the direct load textures.
			inki::texture_output_png *pngObj = inki::texture_output_png::alloc();
			pngObj->u0 = u0;
			pngObj->v0 = v0;
			pngObj->u1 = u1;
			pngObj->v1 = v1;
			
			RECORD_INFO(record, "[TextureOutputFormatPng] - Source image [" << png.width << "x" << png.height << "] => [" << texture->width << "x" << texture->height << "]")
			
			ccgui::pngutil::write_buffer wb = ccgui::pngutil::write_to_mem(outData, out_width, out_height, ((inki::texture_output_format_png*)outputFormat)->compression_level);
			
			texture->output = pngObj;
			texture->output->data = inki::data_container::alloc();
			texture->output->data->config = outputFormat->storage_configuration;
			texture->output->data->bytes.insert(texture->output->data->bytes.begin(), (unsigned char*)wb.output, (unsigned char*)(wb.output + wb.size));
			texture->output->data->file_type = "png";

			::free(wb.output);
			
			add_output(context, record, path_res.c_str(), pngObj);
			add_output(context, record, path_res_data.c_str(), texture->output->data);
		}
		else if (outputFormat->rtti_type_ref() == inki::texture_output_format_jpeg::type_id())
		{
			// these are the direct load textures.
			inki::texture_output_jpeg *jpgObj = inki::texture_output_jpeg::alloc();
			jpgObj->u0 = u0;
			jpgObj->v0 = v0;
			jpgObj->u1 = u1;
			jpgObj->v1 = v1;
			texture->output = jpgObj;

			int buf_size = 4*1024*1024;
			char *databuffer = new char[buf_size];

			// swap order in-place in the png buffer (naughty)
			unsigned char *pngpixels = (unsigned char*)outData;
			for (unsigned int i=0;i<out_width*out_height;i++)
			{
				unsigned char t0 = pngpixels[0];
				unsigned char t1 = pngpixels[1];
				unsigned char t2 = pngpixels[2];
				unsigned char t3 = pngpixels[3];
				pngpixels[0] = t2;
				pngpixels[1] = t1;
				pngpixels[2] = t0;
				pngpixels[3] = t3;

				pngpixels += 4;
			}

			inki::texture_output_format_jpeg *fmt = (inki::texture_output_format_jpeg *) outputFormat;
			jpge::params p;
			p.m_quality = fmt->quality;
			p.m_two_pass_flag = fmt->twopass;

			if (!jpge::compress_image_to_jpeg_file_in_memory(databuffer, buf_size, out_width, out_height, 4, (unsigned char*)outData, p))
			{
				RECORD_ERROR(record, "JPEG compression failed");
				return false;
			}
			RECORD_INFO(record, "[jpeg] compressed " << out_width << "x" << out_height << " to " << buf_size << " bytes.")
			
			texture->output = jpgObj;
			texture->output->data = inki::data_container::alloc();
			texture->output->data->config = outputFormat->storage_configuration;
			texture->output->data->bytes.insert(texture->output->data->bytes.begin(), (unsigned char*)databuffer, (unsigned char*)(databuffer + buf_size));
			texture->output->data->file_type = "jpeg";
			
			add_output(context, record, path_res.c_str(), jpgObj);
			add_output(context, record, path_res_data.c_str(), texture->output->data);
		
			delete [] databuffer;
		}

		if (outData != png.pixels)
		{
			delete [] outData;
		}

		ccgui::pngutil::free(&png);
		return false;
	}
};

void register_texture_builder(putki::builder::data *builder)
{
	static texbuilder fb;
	putki::builder::add_data_builder(builder, "Texture", &fb);
}
