#include "datacontainer/datacontainer.h"
#include "log/log.h"

#include <putki/liveupdate/liveupdate.h>
#include <outki/types/kosmos/Texture.h>
#include <OpenGL/gl.h>

#include "render/shader.h"
#include "render/render.h"

#include <inttypes.h>
#include <stdio.h>
#include <map>
#include <string>
#include <vector>

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
		
		struct state_buf
		{
			float width, height;
			float xs, ys;
			float xofs, yofs;
			
			render::loaded_texture *tex0;
			shader::program *prog;
		};
		
		struct render_block
		{
			state_buf state;
			vertex_t *begin, *end;
		};
		
		struct stream
		{
			GLuint vbo;
			vertex_t *verts;
			size_t bufsize;
			shader::program *prog_solid, *prog_textured;
			state_buf state;
			std::vector<render_block> blocks;
			bool touched_state;
		};
		
		#define SET_STATE(stream, field, value) \
			if (stream->state. field != value) { \
				stream->touched_state = true; \
				stream->state. field = value; \
			} \
				
	
		stream *stream_create(unsigned int triangles)
		{
			stream *s = new stream();
			glGenBuffers(1, &s->vbo);
			s->bufsize = 6 * triangles;
			s->verts = new vertex_t[s->bufsize];

			memset(&s->state, 0x00, sizeof(state_buf));
			s->touched_state = false;
			s->state.xs = s->state.ys = 1;
			
 			return s;
		}
	
		void set_2d_transform(stream *s, float xs, float ys, float xofs, float yofs)
		{
			SET_STATE(s, xs, xs)
			SET_STATE(s, ys, ys)
			SET_STATE(s, xofs, xofs)
			SET_STATE(s, yofs, yofs)
		}
			
		void stream_free(stream *s)
		{
			glDeleteBuffers(1, &s->vbo);
			delete [] s->verts;
			delete s;
		}
		
		void submit_block(stream *s, render_block *block)
		{
			float mtx[16] = {0};
			mtx[0] = mtx[5] = mtx[10] = mtx[15] = 1.0f;
			
			state_buf *state = &block->state;
			
			mtx[0] = (2.0f / state->width) * state->xs;
			mtx[5] = -(2.0f / state->height) * state->ys;
			
			mtx[12] = -1.0f + (state->xofs * mtx[0]);
			mtx[13] = 1 + (state->yofs * mtx[5]);
			
			shader::program_use(state->prog);
			
			glUniformMatrix4fv(shader::find_uniform(state->prog, "proj"), 1, GL_FALSE, mtx);
			
			vertex_t *v0 = 0;
			void *uv0 = &v0->u0;
			void *col = &v0->color;
			
			glVertexAttribPointer(shader::find_attribute(state->prog, "vpos"), 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), 0);
			glVertexAttribPointer(shader::find_attribute(state->prog, "uv0"), 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), uv0);
			glVertexAttribPointer(shader::find_attribute(state->prog, "color"), 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(vertex_t), col);
			
			glEnableVertexAttribArray(shader::find_attribute(state->prog, "vpos"));
			glEnableVertexAttribArray(shader::find_attribute(state->prog, "uv0"));
			glEnableVertexAttribArray(shader::find_attribute(state->prog, "color"));
			
			if (state->tex0)
			{
				glEnable(GL_TEXTURE_2D);
				glUniform1i(shader::find_uniform(state->prog, "texture"), 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, render::tex_id(state->tex0));
			}
			else
			{
				glDisable(GL_TEXTURE_2D);
			}
									
			glEnable(GL_BLEND);
			glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			
			glDrawArrays(GL_TRIANGLES, block->begin - s->verts, block->end - block->begin);
		}
		
		void submit(stream *s)
		{
			if (s->blocks.empty())
				return;
				
			// all the vertices for these blocks.
			glBindBuffer(GL_ARRAY_BUFFER, s->vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_t) * (s->blocks.back().end - s->blocks.front().begin), s->verts, GL_STREAM_DRAW);
		
			for (int i=0;i!=s->blocks.size();i++)
				submit_block(s, &s->blocks[i]);

			s->blocks.clear();
		}
		
		vertex_t *start_insert(stream *s, unsigned int count)
		{
			if (s->blocks.empty())
			{
				// start new block with current state
				render_block tmp;
				s->blocks.push_back(tmp);
				render_block & rb = s->blocks.back();
				rb.state = s->state;
				rb.begin = s->verts;
				rb.end = s->verts + count;
				if (count > s->bufsize)
				{
					KOSMOS_ERROR("No room!");
					return 0;
				}
				s->touched_state = false;
				return s->verts;
			}
			
			render_block *rb = &s->blocks.back();
		
			// no room?
			if (rb->end + count > (s->verts + s->bufsize))
			{
				submit(s);
				return start_insert(s, count);
			}
			
			if (!s->touched_state || !memcmp(&s->state, &rb->state, sizeof(state_buf)))
			{
				// continue on the same block since they have same state
				s->touched_state = false;
				vertex_t *ins = rb->end;
				rb->end += count;
				return ins;
			}
			
			// state is different but there is room
			
			render_block tmp;
			s->blocks.push_back(tmp);
			render_block & nb = s->blocks.back();
			nb.state = s->state;
			nb.begin = rb->end;
			nb.end = nb.begin + count;
			s->touched_state = false;
			return nb.begin;
		}
		
		void set_viewport(stream *s, float width, float height)
		{
			SET_STATE(s, width, width)
			SET_STATE(s, height, height)
		}
		
		void use_programs(stream *s, shader::program *prog_solid, shader::program *prog_tex)
		{
			s->prog_solid = prog_solid;
			s->prog_textured = prog_tex;
		}
				
		void stream_done(stream *s)
		{
			submit(s);
		}
	
		void gradient_rect(stream *s, float x0, float y0, float x1, float y1, unsigned int tl, unsigned int tr, unsigned int bl, unsigned int br)
		{
			SET_STATE(s, prog, s->prog_solid)
			SET_STATE(s, tex0, 0)
			
			vertex_t *out = start_insert(s, 6);

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
		}
		
		void tex_rect(stream *s, render::loaded_texture *tex, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, unsigned int color)
		{
			SET_STATE(s, prog, s->prog_textured)
			SET_STATE(s, tex0, tex)
					
			vertex_t *out = start_insert(s, 6);
		
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
		}
	
		void screen_to_local(stream *s, float x, float y, float *xout, float *yout)
		{
			*xout = (x / s->state.xs) - s->state.xofs;
			*yout = (y / s->state.ys) - s->state.yofs;
		}
		
		void solid_rect(stream *s, float x0, float y0, float x1, float y1, unsigned int color)
		{
			gradient_rect(s, x0, y0, x1, y1, color, color, color, color);
		}
	}
}
	