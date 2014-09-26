#ifndef __KOSMOS_RENDER2D_H__
#define __KOSMOS_RENDER2D_H__

namespace kosmos
{
	namespace shader
	{
		struct program;
	}
	
	namespace render2d
	{
		struct stream;
			
		stream *stream_create(unsigned int triangles);
		void stream_free(stream *);
		void stream_done(stream *);
		
		void use_programs(stream *s, shader::program *solid, shader::program *textured);
		void set_viewport(stream *s, float width, float height);
		void set_2d_transform(stream *s, float xs, float ys, float xofs, float yofs);
		
		void tex_rect(stream *s, render::loaded_texture *tex, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, unsigned int color);
		void solid_rect(stream *s, float x0, float y0, float x1, float y1, unsigned int color);
		void gradient_rect(stream *s, float x0, float y0, float x1, float y1, unsigned int tl, unsigned int tr, unsigned int bl, unsigned int br);
	
	}
}

#endif
