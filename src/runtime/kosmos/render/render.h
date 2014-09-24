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
	
		// Workflow.
		void begin(int width, int height, bool clearcolor, bool cleardepth, unsigned int clear_color);
		void end();
		
		void reset_matrices_ortho(int width, int height);

		void solid_rect(float x0, float y0, float x1, float y1, unsigned int color);
		void gradient_rect(float x0, float y0, float x1, float y1, unsigned int tl, unsigned int tr, unsigned int bl, unsigned int br);
		void tex_rect(loaded_texture *tex, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, unsigned int color);
		void line(float x0, float y0, float x1, float y1, unsigned int color);
	}
}

#endif
