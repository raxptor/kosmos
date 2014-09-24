#include "datacontainer/datacontainer.h"
#include "log/log.h"

#include <putki/liveupdate/liveupdate.h>
#include <outki/types/kosmos/Texture.h>
#include <OpenGL/gl.h>

#include <stdio.h>
#include <map>
#include <string>

namespace kosmos
{
	namespace render
	{
		void get_client_rect(int *x0, int *y0, int *x1, int *y1)
		{
			*x0 = *y0 = 0;
			*x1 = *y1 = 100;
		}
		
		struct loaded_texture
		{
			outki::DataContainer *container;
			outki::Texture *source_tex;
			GLuint handle;
			std::string source;
			int refcount;
		};

		typedef std::map<std::string, loaded_texture*> LoadedTextures;

		namespace
		{
			LoadedTextures s_textures;
		};

		void unload_texture(loaded_texture *tex)
		{
			if (tex->refcount-- == 0)
			{
				glDeleteTextures(1, &tex->handle);
				s_textures.erase(s_textures.find(tex->source));
				delete tex;
			}
		}

		void empty_texture(GLuint tex)
		{
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST  );
			unsigned char pixels[] = {0xff,0xff,0xff,0xff};
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
				      0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		void update_texture(loaded_texture *tex)
		{
			datacontainer::loaded_data *loaded = datacontainer::load(tex->container, true);
			if (!loaded || !loaded->size)
			{
				empty_texture(tex->handle);
				if (loaded) datacontainer::release(loaded);
				return;
			}

			if (!loaded->size)
			{
				KOSMOS_WARNING("Empty texture?!")
				empty_texture(tex->handle);
				if (loaded) datacontainer::release(loaded);
				return;
			}

			switch (tex->source_tex->Output->rtti_type_ref())
			{
				case outki::TextureOutputRaw::TYPE_ID:
				{
					if (loaded->size != 4 * tex->source_tex->Width * tex->source_tex->Height)
					{
						KOSMOS_WARNING("Texture is " << tex->source_tex->Width << "x" << tex->source_tex->Height << " but bytes are " << loaded->size)
						empty_texture(tex->handle);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, tex->handle);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR  );
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->source_tex->Width, tex->source_tex->Height,
							      0, GL_RGBA, GL_UNSIGNED_BYTE, loaded->data);
						glBindTexture(GL_TEXTURE_2D, 0);
						KOSMOS_INFO("Updated texture [" << tex->handle << "] with " << loaded->size << " bytes");
					}
					break;
				}
				default:
				{
					empty_texture(tex->handle);
					KOSMOS_WARNING("Unsupported texture format");
					break;
				}
			}

			datacontainer::release(loaded);
		}


		loaded_texture * load_texture(outki::Texture *texture)
		{
			LoadedTextures::iterator i = s_textures.find(texture->id);
			if (i != s_textures.end())
			{
				i->second->refcount++;
				return i->second;
			}

			if (!texture->Output)
			{
				KOSMOS_WARNING("Trying to load a texture which has no generated output! [" << texture->id << "]");
				return 0;
			}

			loaded_texture *tex = new loaded_texture();
			tex->refcount = 1;
			tex->source = texture->id;
			tex->container = texture->Output->Data;
			tex->source_tex = texture;
			glGenTextures(1, &tex->handle);

			s_textures.insert(LoadedTextures::value_type(texture->id, tex));

			update_texture(tex);
			return tex;
		}
		
		void reset_matrices_ortho(int width, int height)
		{
			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();
			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			glOrtho(0, (float)width, (float)height, 0, -1, 1);
		}

		void begin(int width, int height, bool clearcolor, bool cleardepth, unsigned int clear_color)
		{
			//
			LoadedTextures::iterator i = s_textures.begin();
			while (i != s_textures.end())
			{
				if (LIVE_UPDATE(&i->second->container))
				{
					KOSMOS_INFO("Texture live updated");
					update_texture(i->second);
				}

				++i;
			}
			
			glViewport(0, 0, width, height);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
			
			reset_matrices_ortho(width, height);
		}

		void end()
		{
			glFlush();
		}

		bool get_size(int * width, int * height)
		{
			int x0, y0, x1, y1;
			get_client_rect(&x0, &y0, &x1, &y1);
			*width = x1 - x0;
			*height = y1 - y0;
			return true;
		}

		inline void intColor(unsigned int color)
		{
			glColor4ub((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff, (color >> 24)&0xff);
		}

		void gradient_rect(float x0, float y0, float x1, float y1, unsigned int tl, unsigned int tr, unsigned int bl, unsigned int br)
		{
			glBegin(GL_TRIANGLE_STRIP);
			intColor(tl);
			glVertex2f(x0,y0);
			intColor(tr);
			glVertex2f(x1,y0);
			intColor(bl);
			glVertex2f(x0,y1);
			intColor(br);
			glVertex2f(x1,y1);
			glEnd();
		}

		void solid_rect(float x0, float y0, float x1, float y1, unsigned int color)
		{
			intColor(color);
			glBegin(GL_TRIANGLE_STRIP);
			glVertex2f(x0,y0);
			glVertex2f(x1,y0);
			glVertex2f(x0,y1);
			glVertex2f(x1,y1);
			glEnd();
		}

		void line(float x0, float y0, float x1, float y1, unsigned int color)
		{
			glLineWidth(1);
			intColor(color);
			glBegin(GL_LINES);
			glVertex2f(x0,y0);
			glVertex2f(x1,y1);
			glEnd();
		}

		void tex_rect(loaded_texture *tex, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, unsigned int color)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, tex->handle);

			glColor4ub((color >> 16) & 0xff, (color >> 8) & 0xff, color & 0xff, (color >> 24)&0xff);
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(u0, v0);
			glVertex2f(x0,y0);
			glTexCoord2f(u1, v0);
			glVertex2f(x1,y0);
			glTexCoord2f(u0, v1);
			glVertex2f(x0,y1);
			glTexCoord2f(u1, v1);
			glVertex2f(x1,y1);
			glEnd();

			glDisable(GL_TEXTURE_2D);
		}

	}
}