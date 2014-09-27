#ifndef __CLAW_RENDER_H___
#define __CLAW_RENDER_H___

namespace outki { struct Texture; }

namespace kosmos
{
	namespace render
	{
		// Texture handling.
		struct loaded_texture;
		
		loaded_texture * load_texture(outki::Texture *texture);
		void unload_texture(loaded_texture *tex);
		int tex_id(loaded_texture *tex);
	
		// Workflow.
		void begin(int width, int height, bool clearcolor, bool cleardepth, unsigned int clear_color);
		void end();
	}
}

#endif
