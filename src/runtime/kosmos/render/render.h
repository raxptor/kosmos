#ifndef __CLAW_RENDER_H___
#define __CLAW_RENDER_H___

namespace outki { struct texture; }

namespace kosmos
{
	namespace render
	{
		// Texture handling.
		struct texture_ref;

		// load texture bundled with kosmos data, might return existing instance
		// with bumped refcount.
		texture_ref * load_texture(outki::texture *texture);

		// handle an opengl texture for us
		texture_ref * make_ref(int handle);

		int tex_id(texture_ref *);

		// free resources. (decrease refcount)
		void unload_texture(texture_ref *tex);

		// convenience function to return the textures output file path, if
		// actually exists.
		const char *get_texture_file_path(outki::texture *texture);
	
		// Workflow.
		void begin(int width, int height, bool clearcolor, bool cleardepth, unsigned int clear_color);
		void end();
	}
}

#endif
