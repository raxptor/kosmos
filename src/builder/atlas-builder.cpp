#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/build-db.h>
#include <putki/builder/log.h>

#include "kosmos-builder-utils/binpacker/maxrects_binpack.h"
#include "kosmos-builder-utils/pngutil.h"
#include "kosmos-builder-utils/textureconfig.h"

#include <inki/types/kosmos/Texture.h>
#include <inki/types/kosmos/Atlas.h>

#include <iostream>
#include <vector>
#include <sstream>
#include <math.h>

namespace
{
	#define KERNEL_SIZE 5
	struct sample_kernel
	{
		float k[KERNEL_SIZE * KERNEL_SIZE];
	};

	void make_sample_kernel(sample_kernel *out, float rt, float suppression = 0.90f, float adjx=0, float adjy=0)
	{
		const float d = sqrt(-2 * logf(suppression)) / (rt * 2.0f * 3.1415f);
		const float mul = 1.0f / sqrtf(2.0f * 3.1415f * d);

		float sum = 0;
		for (int x=0;x<KERNEL_SIZE;x++)
		{
			for (int y=0;y<KERNEL_SIZE;y++)
			{
				const float cx = x - KERNEL_SIZE / 2 - adjx;
				const float cy = y - KERNEL_SIZE / 2 - adjy;
				const float val = mul * expf( -float(cx*cx + cy*cy) / (2.0f * d *d));
				out->k[y*KERNEL_SIZE+x] = val;
				sum += val;
			}
		}

		for (int x=0;x<KERNEL_SIZE*KERNEL_SIZE;x++)
			out->k[x] /= sum;
	}

	// point sampling.
	unsigned long sample(const unsigned int * px, int s_width, int s_height, int t_width, int t_height, const sample_kernel &kernel, int x, int y)
	{
		float _x = float(x) * float(s_width) / float(t_width);
		float _y = float(y) * float(s_height) / float(t_height);

		sample_kernel blah;
		make_sample_kernel(&blah, float(t_width) / float(s_width), 0.05f, _x - floorf(_x), _y - floorf(_y));

		unsigned int outpx = 0;
		for (int component=0;component<32;component+=8)
		{
			int cx = int(_x) - KERNEL_SIZE / 2;
			int cy = int(_y) - KERNEL_SIZE / 2;

			unsigned char *pxc = (unsigned char*) px;
			pxc += component / 8;

			float sum = 0;
			for (int y=0;y<KERNEL_SIZE;y++)
			{
				for (int x=0;x<KERNEL_SIZE;x++)
				{
					int px = cx + x;
					int py = cy + y;
					if (px < 0)
					{
						px = 0;
					}
					if (py < 0)
					{
						py = 0;
					}
					if (px > s_width - 1)
					{
						px = s_width  - 1;
					}
					if (py > s_height - 1)
					{
						py = s_height - 1;
					}
					sum += float(pxc[4 * (py * s_width + px)]) * blah.k[y*KERNEL_SIZE+x];
				}
			}

			if (sum > 255)
			{
				sum = 255;
			}
			if (sum < 0)
			{
				sum = 255;
			}
			int val = (int) sum;
			outpx |= (val << component);
		}

		return outpx;

		//return px[s_width * y + x];
	}

	bool build_atlas(const putki::builder::build_info* info)
	{		
		inki::atlas *atlas = (inki::atlas *) info->object;

		std::vector<kosmos::pngutil::loaded_png> loaded;
		std::vector<rbp::InputRect> inputRects;

		int max_width = 1;
		int max_height = 1;

		int border = 2;

		if (atlas->inputs.size() == 1)
		{
			border = 0;
		}

		for (unsigned int i=0;i<atlas->inputs.size();i++)
		{
			if (!atlas->inputs[i].get())
			{
				RECORD_WARNING(info->record, "Blank entry in atlas at slot " << i)
				continue;
			}

			kosmos::pngutil::loaded_png png;
			if (!kosmos::pngutil::load_from_resource(info, atlas->inputs[i]->source.c_str(), &png))
			{
				RECORD_WARNING(info->record, "Failed to load png");
			}
			else
			{		
				loaded.push_back(png);

				rbp::InputRect ir;
				ir.width = png.width;
				ir.height = png.height;
				ir.id = loaded.size() - 1;
				inputRects.push_back(ir);

				if (ir.width > max_width)
				{
					max_width = ir.width;
				}
				if (ir.height > max_height)
				{
					max_height = ir.height;
				}
				RECORD_INFO(info->record, " - " << atlas->inputs[i]->source.c_str() << " loaded [" << png.width << "x" << png.height << "]")
			}
		}

		for (int i=0;i<g_outputTexConfigs;i++)
		{
			int out_width = 1;
			int out_height = 1;

			const TextureScaleConf &scaleConfig = g_outputTexConf[i];

			const int m_w = (int)ceilf(max_width * scaleConfig.scale);
			const int m_h = (int)ceilf(max_height * scaleConfig.scale);

			// start values that can actually contain the items.
			while (out_width  < m_w) out_width *= 2;
			while (out_height < m_h) out_height *= 2;

			std::vector<rbp::Rect> packedRects;

			// pack until we know how to do it!
			while (true)
			{
				rbp::MaxRectsBinPack pack(out_width, out_height);

				std::vector< rbp::InputRect > tmpCopy = inputRects;
				for (unsigned int i=0;i<tmpCopy.size();i++)
				{
					tmpCopy[i].width  = (int)floorf(0.5f + tmpCopy[i].width * scaleConfig.scale) + 2 * border;
					tmpCopy[i].height = (int)floorf(0.5f + tmpCopy[i].height * scaleConfig.scale) + 2 * border;
				}

				pack.Insert(tmpCopy, packedRects, rbp::MaxRectsBinPack::RectBottomLeftRule);

				if (packedRects.size() == inputRects.size())
				{
					break;
				}
				else
				{
					if (out_height > out_width)
					{
						out_width *= 2;
					}
					else
					{
						out_height *= 2;
					}

					packedRects.clear();
				}
			}

			// make the atlas.
			unsigned int * outBmp = new unsigned int[out_width * out_height];
			memset(outBmp, 0x00, sizeof(unsigned int) * out_width * out_height);

			/* - test pattern -
			   for (int y=0;y<out_height;y++)
			   {
			        for (int x=0;x<out_width;x++)
			        {
			                outBmp[y*out_width+x] = (x^y) & 1 ? 0xff101010 : 0xfff0f0f0;
			        }
			   }
			 */

			inki::atlas_output ao;

			ao.width = out_width;
			ao.height= out_height;
			ao.scale = g_outputTexConf[i].scale;

			RECORD_INFO(info->record, "Packing into " << out_width << "x" << out_height)

			for (unsigned int k=0;k<packedRects.size();k++)
			{
				kosmos::pngutil::loaded_png const &g = loaded[packedRects[k].id];
				rbp::Rect const &out = packedRects[k];

				sample_kernel krn;
				make_sample_kernel(&krn, scaleConfig.scale);

				for (int y=0;y<out.height;y++)
				{
					for (int x=0;x<out.width;x++)
						outBmp[out_width * (out.y + y) + (out.x + x)] = sample(g.pixels, g.width, g.height, out.width - border * 2, out.height - border * 2, krn, x - border, y - border);
				}

				inki::atlas_entry e;
				e.id = atlas->inputs[packedRects[k].id].path();
				e.u0 = float(out.x + border) / float(out_width);
				e.v0 = float(out.y + border) / float(out_height);
				e.u1 = float(out.x + out.width - border) / float(out_width);
				e.v1 = float(out.y + out.height - border) / float(out_height);
				ao.entries.push_back(e);
			}

			kosmos::pngutil::write_buffer wb = kosmos::pngutil::write_to_mem(outBmp, out_width, out_height, 2);
			std::string output_atlas_path = putki::builder::store_resource_tag(info, "out.png", wb.output, wb.size);
			::free(wb.output);

			{
				// create new texture.
				putki::ptr<inki::texture> texture = putki::builder::create_build_output<inki::texture>(info, "out-tex");
				texture->source = output_atlas_path;
				texture->configuration = atlas->output_configuration;
				ao.texture = texture;
				atlas->outputs.push_back(ao);
			}

			for (unsigned int i=0;i!=loaded.size();i++)
				kosmos::pngutil::free(&loaded[i]);

			delete [] outBmp;
		}
		return true;
	}
};

void register_atlas_builder(putki::builder::data *builder)
{
	putki::builder::handler_info info[1] = {
		{ inki::atlas::type_id(), "atlas-builder-5", build_atlas, 0 }
	};
	putki::builder::add_handlers(builder, &info[0], &info[1]);
}
