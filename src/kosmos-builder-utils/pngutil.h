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

		struct loaded_png
		{
			unsigned int *pixels;
			unsigned int width, height;
			unsigned int bpp; // always 32 for now
		};
        
		bool load_from_resource(const putki::builder::build_info* info, const char* path, loaded_png* out);
        bool load_info_from_resource(const putki::builder::build_info* info, const char* path, loaded_png* out);
        
        write_buffer write_to_mem(unsigned int *pixbuf, unsigned int width, unsigned int height, int compression_level=1);
		void free(loaded_png *png);
	}
}
