#include "datacontainer/datacontainer.h"
#include "log/log.h"

#include <putki/liveupdate/liveupdate.h>
#include <outki/types/kosmos/Texture.h>

#include "glwrap/gl.h"

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
		
		struct texture_ref
		{
			outki::DataContainer *container;
			outki::Texture *source_tex;
			GLuint handle;
			std::string source;
			int refcount;
		};

		typedef std::map<std::string, texture_ref*> LoadedTextures;

		namespace
		{
			LoadedTextures s_textures;
		};

		void unload_texture(texture_ref *tex)
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
			unsigned char pixels[] = {0xff,0x00,0xff,0xff};
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1,
				      0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		const char *get_texture_file_path(outki::Texture *texture)
		{
			if (!texture) return 0;
			if (!texture->Output) return 0;
			if (!texture->Output->Data) return 0;
			if (!texture->Output->Data->Output) return 0;
			outki::DataContainerOutputFile *of = texture->Output->Data->Output->exact_cast<outki::DataContainerOutputFile>();
			if (!of) return 0;
			return of->FilePath;
		}

		void update_texture(texture_ref *tex)
		{
			if (LIVEUPDATE_ISNULL(tex->container))
			{
				empty_texture(tex->handle);
				return;
			}

			datacontainer::loaded_data *loaded = datacontainer::load(tex->container, true);
			if (!loaded || !loaded->size)
			{
				empty_texture(tex->handle);
				if (loaded) datacontainer::release(loaded);
				return;
			}

			if (!loaded->size || !tex->source_tex->Output)
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

		texture_ref * make_ref(int handle)
		{
			texture_ref *tex = new texture_ref();
			tex->refcount = 1;
			tex->container = 0;
			tex->source_tex = 0;
			tex->handle = handle;
			return tex;
		}

		texture_ref * load_texture(outki::Texture *texture)
		{
			LoadedTextures::iterator i = s_textures.find(texture->id);
			if (i != s_textures.end())
			{
				i->second->refcount++;
				return i->second;
			}

			LIVE_UPDATE(&texture);

			if (!texture->Output)
			{
				KOSMOS_WARNING("Trying to load a texture which has no generated output! [" << texture->id << "]");
				return 0;
			}

			texture_ref *tex = new texture_ref();
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
			/*
			glMatrixMode( GL_MODELVIEW );
			glLoadIdentity();
			glMatrixMode( GL_PROJECTION );
			glLoadIdentity();
			glOrtho(0, (float)width, (float)height, 0, -1, 1);
			*/
		}

		void transform_2d(float scalex, float scaley, float ofsx, float ofsy)
		{
		//	glScalef(scalex, scaley, 1);
		//	glTranslatef(ofsx, ofsy, 0);
		}

		void push_matrix()
		{
		//	glPushMatrix();
		}
		
		void pop_matrix()
		{
		//	glPopMatrix();
		}

		void begin(int width, int height, bool clearcolor, bool cleardepth, unsigned int clear_color)
		{
			//
			LoadedTextures::iterator i = s_textures.begin();
			while (i != s_textures.end())
			{
				bool upd = false;
				if (LIVE_UPDATE(&i->second->source_tex))
				{
					if (i->second->source_tex->Output)
						i->second->container = i->second->source_tex->Output->Data;
					else
						i->second->container = 0;
					upd = true;
				}

				if (i->second->container)
				{
					if (LIVE_UPDATE(&i->second->container))
						upd = true;
					if (LIVE_UPDATE(&i->second->container->Output))
						upd = true;
				}

				if (upd)
				{
					KOSMOS_INFO("Texture live updated");
					update_texture(i->second);
				}

				++i;
			}
			
			glViewport(0, 0, width, height);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			
			glEnableClientState(GL_VERTEX_ARRAY);
			
			static GLuint VertexArrayID = -1;
			if (VertexArrayID == -1)
				glGenVertexArraysAPPLE(1, &VertexArrayID);
			glBindVertexArrayAPPLE(VertexArrayID);

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

		void line(float x0, float y0, float x1, float y1, unsigned int color)
		{
			glLineWidth(1);
			intColor(color);
			glBegin(GL_LINES);
			glVertex2f(x0,y0);
			glVertex2f(x1,y1);
			glEnd();
		}

		int tex_id(texture_ref *tex)
		{
			return tex->handle;
		}
	}
}