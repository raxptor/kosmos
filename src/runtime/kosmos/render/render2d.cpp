#include "datacontainer/datacontainer.h"
#include "log/log.h"

#include <putki/liveupdate/liveupdate.h>
#include <outki/types/kosmos/Texture.h>
#include <OpenGL/gl.h>

#include <render/shader.h>
#include <render/render.h>

#include <inttypes.h>
#include <stdio.h>
#include <map>
#include <string>

namespace kosmos
{
	namespace render2d
	{
		struct vertex_t
		{
			float x, y;
			float u0, v0;
			unsigned int color;
		};
		
		struct stream
		{
			GLuint vbo;
			vertex_t *verts, *writeptr;
			size_t bufsize;
			
			float width, height;
			float xs, ys;
			float xofs, yofs;
			
			shader::program *prog_solid, *prog_textured, *prog;
			render::loaded_texture *tex0;
		};
	
		stream *stream_create(unsigned int triangles)
		{
			stream *s = new stream();
			glGenBuffers(1, &s->vbo);
			s->bufsize = 6 * triangles;
			s->verts = new vertex_t[s->bufsize];
			s->writeptr = s->verts;
			s->prog_solid = s->prog_textured = s->prog = 0;
			s->tex0 = 0;
			
			s->width = s->height = 1024;
			s->xs = s->ys = 1;
			s->xofs = s->yofs = 0;;
 			return s;
		}
	
		void set_2d_transform(stream *s, float xs, float ys, float xofs, float yofs)
		{
			s->xs = xs;
			s->ys = ys;
			s->xofs = xofs;
			s->yofs = yofs;
		}
			
		void stream_free(stream *s)
		{
			glDeleteBuffers(1, &s->vbo);
			delete [] s->verts;
			delete s;
		}
		
		void submit(stream *s)
		{
			if (s->writeptr == s->verts)
				return;

			float mtx[16] = {0};
			mtx[0] = mtx[5] = mtx[10] = mtx[15] = 1.0f;
			
			mtx[0] = (2.0f / s->width) * s->xs;
			mtx[5] = -(2.0f / s->height) * s->ys;
			
			mtx[12] = -1.0f + (s->xs * mtx[0]);
			mtx[13] = 1 - (s->ys * mtx[0]);
			
			shader::program_use(s->prog);
			glUniformMatrix4fv(shader::find_uniform(s->prog, "proj"), 1, GL_FALSE, mtx);

			glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * (s->writeptr - s->verts), s->verts, GL_STREAM_DRAW);
			
			vertex_t *v0 = 0;
			void *uv0 = &v0->u0;
			void *col = &v0->color;
			
			glVertexAttribPointer(shader::find_attribute(s->prog, "vpos"), 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
			glVertexAttribPointer(shader::find_attribute(s->prog, "uv0"), 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), uv0);
			glVertexAttribPointer(shader::find_attribute(s->prog, "color"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex_t), col);
			
			glEnableVertexAttribArray(shader::find_attribute(s->prog, "vpos"));
			glEnableVertexAttribArray(shader::find_attribute(s->prog, "uv0"));
			glEnableVertexAttribArray(shader::find_attribute(s->prog, "color"));
			
			if (s->tex0)
			{
				glEnable(GL_TEXTURE_2D);
				glUniform1i(shader::find_uniform(s->prog, "texture"), 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, render::tex_id(s->tex0));
				kosmos::shader::program_use(s->prog_textured);
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
				kosmos::shader::program_use(s->prog_solid);
			}
						
			glEnable(GL_BLEND);
			glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			
			glDrawArrays(GL_TRIANGLES, 0, s->writeptr - s->verts);
			s->writeptr = s->verts;
		}
		
		void set_viewport(stream *s, float width, float height)
		{
			submit(s);
			s->width = width;
			s->height = height;
		}
		
		void use_programs(stream *s, shader::program *prog_solid, shader::program *prog_tex)
		{
			s->prog_solid = prog_solid;
			s->prog_textured = prog_tex;
			submit(s);
		}
				
		void stream_done(stream *s)
		{
			submit(s);
		}
	
		void gradient_rect(stream *s, float x0, float y0, float x1, float y1, unsigned int tl, unsigned int tr, unsigned int bl, unsigned int br)
		{
			if ((s->writeptr + 4) > (s->verts + s->bufsize))
				submit(s);
		
			vertex_t *out = s->writeptr;
			
			s->prog = s->prog_solid;
			s->tex0 = 0;

			out[0].x = x0;
			out[0].y = y0;
			out[0].u0 = out[0].v0 = 0;
			out[0].color = tl;

			out[1].x = x1;
			out[1].y = y0;
			out[1].u0 = out[1].v0 = 0;
			out[1].color = tr;

			out[2].x = x0;
			out[2].y = y1;
			out[2].u0 = out[1].v0 = 0;
			out[2].color = bl;

			out[3].x = x1;
			out[3].y = y0;
			out[3].u0 = out[0].v0 = 0;
			out[3].color = tr;

			out[4].x = x0;
			out[4].y = y1;
			out[4].u0 = out[1].v0 = 0;
			out[4].color = bl;
			
			out[5].x = x1;
			out[5].y = y1;
			out[5].u0 = out[1].v0 = 0;
			out[5].color = br;
			
			s->writeptr += 6;
		}
		
		void tex_rect(stream *s, render::loaded_texture *tex, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, unsigned int color)
		{
			submit(s);
			
			s->prog = s->prog_textured;
			s->tex0 = tex;
			
			vertex_t *out = s->writeptr;
		

			out[0].x = x0;
			out[0].y = y0;
			out[0].u0 = u0;
			out[0].v0 = v0;
			out[0].color = color;

			out[1].x = x1;
			out[1].y = y0;
			out[1].u0 = u1;
			out[1].v0 = v0;
			out[1].color = color;

			out[2].x = x0;
			out[2].y = y1;
			out[2].u0 = u0;
			out[2].v0 = v1;
			out[2].color = color;

			out[3].x = x1;
			out[3].y = y0;
			out[3].u0 = u1;
			out[3].v0 = v0;
			out[3].color = color;

			out[4].x = x0;
			out[4].y = y1;
			out[4].u0 = u0;
			out[4].v0 = v1;
			out[4].color = color;
			
			out[5].x = x1;
			out[5].y = y1;
			out[5].u0 = u1;
			out[5].v0 = v1;
			out[5].color = color;

			s->writeptr += 6;
		}
	
		
		void solid_rect(stream *s, float x0, float y0, float x1, float y1, unsigned int color)
		{
			gradient_rect(s, x0, y0, x1, y1, color, color, color, color);
		}
	}
}
	