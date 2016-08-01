#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/log.h>
#include <putki/builder/build-db.h>

#include "kosmos-builder-utils/pngutil.h"
#include "kosmos-builder-utils/jpge.h"
#include "kosmos-builder-utils/textureconfig.h"

#include <inki/types/kosmos/texture.h>

#include <iostream>

namespace
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
		while (*out_height < height) *out_height *= 2;

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


	bool build_texture(const putki::builder::build_info* info)
	{
		inki::texture *texture = (inki::texture *) info->object;

		// this is used for atlas lookups later.
		texture->id = info->path;

		// this object supplies its own defaults on initialisation
		inki::texture_configuration config;
		if (texture->configuration.get())
		{
			config = *texture->configuration;
		}

		// First load the information for the texture and fill in width & height
		kosmos::pngutil::loaded_png pnginfo;
		if (kosmos::pngutil::load_info_from_resource(info, texture->source.c_str(), &pnginfo))
		{
			texture->width = texture->source_width = pnginfo.width;
			texture->height = texture->source_height = pnginfo.height;
			kosmos::pngutil::free(&pnginfo);
		}
		else
		{
			RECORD_WARNING(info->record, "Could not load source file!");
			return false;
		}

		const inki::texture_output_format *outputFormat = config.output_format(info->build_config).get();

		// If no output is needed
		if (!outputFormat)
		{
			RECORD_INFO(info->record, "No output format, skipping generation")
            return true;
		}
        
        if (!kosmos::pngutil::load_from_resource(info, texture->source.c_str(), &pnginfo))
        {
            RECORD_ERROR(info->record, "png loading failed!");
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

		if (outputFormat->premultiply_alpha)
		{
			for (size_t i = 0; i < pnginfo.width*pnginfo.height; i++)
			{
				unsigned char *ptr = (unsigned char*)&pnginfo.pixels[i];
				ptr[0] = ptr[0] * ptr[3] / 255;
				ptr[1] = ptr[1] * ptr[3] / 255;
				ptr[2] = ptr[2] * ptr[3] / 255;
			}
		}

		// uncrop
		unsigned int *outData = pnginfo.pixels;
		if (out_width != pnginfo.width || out_height != pnginfo.height)
		{
			// only do uncrop
			RECORD_INFO(info->record, "Uncropping to " << out_width << "x" << out_height)
			outData = new unsigned int[out_width * out_height];

			for (int y = 0; y < out_height; y++)
			{
				int srcy = y < pnginfo.height ? y : pnginfo.height - 1;
				unsigned int *srcLine = pnginfo.pixels + pnginfo.width * srcy;
				unsigned int *dstLine = outData + out_width * y;

				for (int x = 0; x < pnginfo.width; x++)
					*dstLine++ = *srcLine++;

				// fill the rest with the last pixel
				for (int x = pnginfo.width; x < out_width; x++)
					*dstLine++ = *(srcLine - 1);
			}
		}

		if (outputFormat->rtti_type_id() == inki::texture_output_format_raw::type_id())
		{
			putki::ptr<inki::data_container> data = putki::builder::create_build_output<inki::data_container>(info, "rawdc");
			data->config = outputFormat->storage_configuration;

			std::vector<unsigned char> & bytesOut = data->bytes;

			// RGBA
			for (int i = 0; i < out_width * pnginfo.height; i++)
			{
				// RGBA
				bytesOut.push_back((outData[i] >> 16) & 0xff);
				bytesOut.push_back((outData[i] >> 8) & 0xff);
				bytesOut.push_back((outData[i] >> 0) & 0xff);
				bytesOut.push_back((outData[i] >> 24) & 0xff);
			}

			putki::ptr<inki::texture_output_raw> raw_tex = putki::builder::create_build_output<inki::texture_output_raw>(info, "raw");
			raw_tex->data = data;
			texture->output = raw_tex;
		}
		else if (outputFormat->rtti_type_id() == inki::texture_output_format_png::type_id())
		{
			RECORD_INFO(info->record, "[TextureOutputFormatPng] - Source image [" << pnginfo.width << "x" << pnginfo.height << "] => [" << texture->width << "x" << texture->height << "]")
			
			kosmos::pngutil::write_buffer wb = kosmos::pngutil::write_to_mem(outData, out_width, out_height, ((inki::texture_output_format_png*)outputFormat)->compression_level);
			putki::ptr<inki::data_container> data = putki::builder::create_build_output<inki::data_container>(info, "pngdc");
			data->config = outputFormat->storage_configuration;
			data->file_type = "png";
			data->bytes.insert(data->bytes.begin(), (unsigned char*)wb.output, (unsigned char*)(wb.output + wb.size));
			::free(wb.output);

			putki::ptr<inki::texture_output_png> png_tex = putki::builder::create_build_output<inki::texture_output_png>(info, "png");
			png_tex->u0 = u0;
			png_tex->v0 = v0;
			png_tex->u1 = u1;
			png_tex->v1 = v1;
			png_tex->data = data;
			texture->output = png_tex;
		}
		else if (outputFormat->rtti_type_id() == inki::texture_output_format_jpeg::type_id())
		{
			int buf_size = 4 * 1024 * 1024;
			char *databuffer = new char[buf_size];

			// swap order in-place in the png buffer (naughty)
			unsigned char *pngpixels = (unsigned char*)outData;
			for (unsigned int i = 0; i < out_width*out_height; i++)
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
				RECORD_ERROR(info->record, "JPEG compression failed");
				return false;
			}
			RECORD_INFO(info->record, "[jpeg] compressed " << out_width << "x" << out_height << " to " << buf_size << " bytes.")

			putki::ptr<inki::data_container> data = putki::builder::create_build_output<inki::data_container>(info, "pngdc");
			data->config = outputFormat->storage_configuration;
			data->file_type = "jpg";
			data->bytes.insert(texture->output->data->bytes.begin(), (unsigned char*)databuffer, (unsigned char*)(databuffer + buf_size));

			// these are the direct load textures.
			putki::ptr<inki::texture_output_jpeg> jpg_tex = putki::builder::create_build_output<inki::texture_output_jpeg>(info, "jpg");
			jpg_tex->u0 = u0;
			jpg_tex->v0 = v0;
			jpg_tex->u1 = u1;
			jpg_tex->v1 = v1;
			jpg_tex->data = data;
			texture->output = jpg_tex;
			delete[] databuffer;
		}

		if (outData != pnginfo.pixels)
		{
			delete[] outData;
		}

		kosmos::pngutil::free(&pnginfo);
		return true;
	}
}

void register_texture_builder(putki::builder::data *builder)
{
	putki::builder::handler_info info[1] = {
		{ inki::texture::type_id(), "texture-builder-1a", build_texture, 0 }
	};
	putki::builder::add_handlers(builder, &info[0], &info[1]);
}
