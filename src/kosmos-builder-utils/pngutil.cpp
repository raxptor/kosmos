#include "pngutil.h"

#include <putki/builder/builder.h>

#include <png.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

namespace kosmos
{
	namespace pngutil
	{
		namespace
		{
			void write(png_structp png_ptr, png_bytep data, png_size_t length)
			{
				write_buffer* p = (write_buffer*) png_get_io_ptr(png_ptr);

				size_t nsize = p->size + length;

				/* allocate or grow buffer */
				if (p->output)
				{
					p->output = (char*)realloc(p->output, nsize);
				}
				else
				{
					p->output = (char*)malloc(nsize);
				}

				/* copy new bytes to end of buffer */
				memcpy(p->output + p->size, data, length);
				p->size += length;
			}

			struct read_buffer
			{
				const putki::builder::build_info* info;
				char buf[128*1024];
				size_t pos, len;
				size_t res_pos;
				const char* path;
			};

			void read(png_structp png_ptr, png_bytep data, png_size_t length)
			{
				read_buffer* p = (read_buffer*) png_get_io_ptr(png_ptr);
				size_t count = p->len - p->pos;
				if (length <= count)
				{
					memcpy(data, &p->buf[p->pos], length);
					p->pos = p->pos + length;
					return;
				}
				if (count > 0)
				{
					memcpy(data, &p->buf[p->pos], count);
					p->pos = 0;
					p->len = 0;
					read(png_ptr, data + count, length - count);
					return;
				}

				if (length > sizeof(p->buf))
				{
					size_t got = putki::builder::read_resource_segment(p->info, p->path, (char*)data, p->res_pos, p->res_pos + length);
					if (got != length)
					{
						png_err(png_ptr);
						return;
					}
					p->res_pos = p->res_pos + length;
				}
				else
				{
					// For when header only is read.
					size_t read_size = p->res_pos < 4096 ? 4096 : sizeof(p->buf);
					p->len = putki::builder::read_resource_segment(p->info, p->path, p->buf, p->res_pos, p->res_pos + read_size);
					p->res_pos = p->res_pos + read_size;
					if (p->len == 0)
					{
						png_err(png_ptr);
						return;
					}
					read(png_ptr, data, length);
				}
			}

			void flush(png_structp png_ptr)
			{

			}
		}

		write_buffer write_to_mem(unsigned int *pixbuf, unsigned int width, unsigned int height, int compression_level)
		{
			write_buffer wb;
			wb.size = 0;
			wb.output = 0;

			png_structp png_ptr = NULL;
			png_infop info_ptr = NULL;
			size_t x, y;
			png_byte ** row_pointers = NULL;
			int status = -1;

			png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
			if (png_ptr == NULL)
			{
				goto png_create_write_struct_failed;
			}

			info_ptr = png_create_info_struct(png_ptr);
			if (info_ptr == NULL)
			{
				goto png_create_info_struct_failed;
			}

			png_set_compression_level(png_ptr, compression_level);

			/* Set image attributes. */
			png_set_IHDR (png_ptr,
					  info_ptr,
					  width,
					  height,
					  8,
					  PNG_COLOR_TYPE_RGBA,
					  PNG_INTERLACE_NONE,
					  PNG_COMPRESSION_TYPE_DEFAULT,
					  PNG_FILTER_TYPE_DEFAULT);

			/* Initialize rows of PNG. */
			row_pointers = (png_byte**) png_malloc(png_ptr, height * sizeof (png_byte *));
			for (y = 0;y < height;++y)
			{
				png_byte *row = (png_byte*) png_malloc(png_ptr, width * 4);
				row_pointers[y] = row;
				for (x = 0;x < width;++x)
				{
					*row++ = (pixbuf[y * width + x] >> 16) & 0xff;
					*row++ = (pixbuf[y * width + x] >> 8) & 0xff;
					*row++ = (pixbuf[y * width + x] >> 0) & 0xff;
					*row++ = (pixbuf[y * width + x] >> 24) & 0xff;
				}
			}

			/* Write the image data to "fp". */

			png_set_write_fn(png_ptr, &wb, &write, &flush);

			png_set_rows (png_ptr, info_ptr, row_pointers);
			png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

			/* The routine has successfully written the file, so we set
			 "status" to a value which indicates success. */

			for (y=0;y<height;y++)
			{
				png_free(png_ptr, row_pointers[y]);
			}

			png_free(png_ptr, row_pointers);
			png_create_info_struct_failed:
			png_destroy_write_struct (&png_ptr, &info_ptr);
			png_create_write_struct_failed:

			return wb;
		}

		bool load(const putki::builder::build_info* info, const char* path, loaded_png *out, bool header_only)
		{
			memset(out, 0x00, sizeof(loaded_png));
			if (!path || !path[0])
			{
				return false;
			}

			png_structp png_ptr;
			png_infop info_ptr;
			unsigned int sig_read = 0;
			int color_type, interlace_type;

			read_buffer rb;
			rb.pos = 0;
			rb.len = 0;
			rb.res_pos = 0;
			rb.info = info;
			rb.path = path;

			png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
			png_set_read_fn(png_ptr, &rb, read);

			info_ptr = png_create_info_struct(png_ptr);
			if (!info_ptr)
			{
				png_destroy_read_struct(&png_ptr, NULL, NULL);
				return false;
			}

			png_set_sig_bytes(png_ptr, sig_read);

			png_uint_32 width, height;
			int bit_depth;

			if (header_only)
			{
				png_read_info(png_ptr, info_ptr);
				png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
				out->width = width;
				out->height = height;
				out->bpp = 32;
				out->pixels = 0;
				png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
				return true;
			}

			png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
			png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

			out->width = width;
			out->height = height;
			out->bpp = 32;

			unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
			out->pixels = (unsigned int *) ::malloc(4 * width * height);

			png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

			for (unsigned int i = 0;i < height;i++)
			{
				unsigned int *outptr = &out->pixels[width * i];
				unsigned char *inptr = row_pointers[i];

				for (unsigned int x=0;x<width;x++)
				{
					if (row_bytes >= width * 4)
					{
						*outptr++ = (inptr[3] << 24) | (inptr[0] << 16) | (inptr[1] << 8) | inptr[2];
						inptr += 4;
					}
					else if (row_bytes >= width * 3)
					{
						*outptr++ = (0xff << 24) | (inptr[0] << 16) | (inptr[1] << 8) | inptr[2];
						inptr += 3;
					}
				}
			}

			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			return true;
		}

		bool load_from_resource(const putki::builder::build_info* info, const char* path, loaded_png* out)
		{
			return load(info, path, out, false);
		}

		bool load_info_from_resource(const putki::builder::build_info* info, const char* path, loaded_png* out)
		{
			return load(info, path, out, true);
		}

		void free(loaded_png *png)
		{
			if (png->pixels)
			{
				::free(png->pixels);
			}
		}
	}
}
