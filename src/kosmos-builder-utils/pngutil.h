#include <string>
#include <putki/builder/builder.h>

namespace kosmos
{
	namespace pngutil
	{
		struct write_buffer
		{
			char *output;
			size_t size;
		};

		// compression level 0=no compression 9=best slowest
		write_buffer write_to_mem(unsigned int *pixbuf, unsigned int width, unsigned int height, int compression_level=1);

		struct loaded_png
		{
			unsigned int *pixels;
			unsigned int width, height;
			unsigned int bpp; // always 32 for now
		};
	
		bool load_from_resource(const putki::builder::build_info* info, const char* path, loaded_png* out);
		void free(loaded_png *png);
	}
}
