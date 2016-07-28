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
			outki::data_container *container;
			outki::texture *source_tex;
			GLuint handle;
			std::string source;
			int refcount;
			bool dynamic;
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

		const char *get_texture_file_path(outki::texture *texture)
		{
			if (!texture) return 0;
			if (!texture->output) return 0;
			if (!texture->output->data) return 0;
			if (!texture->output->data->output) return 0;
			outki::data_container_output_file *of = texture->output->data->output->exact_cast<outki::data_container_output_file>();
			if (!of) return 0;
			return of->file_path;
		}

		void update_texture(texture_ref *tex)
		{
			if (tex->dynamic)
				return;

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

			if (!loaded->size || !tex->source_tex->output)
			{
				KOSMOS_WARNING("Empty texture?!")
				empty_texture(tex->handle);
				if (loaded) datacontainer::release(loaded);
				return;
			}

			switch (tex->source_tex->output->rtti_type_id())
			{
				case outki::texture_output_raw::TYPE_ID:
				{
					if (loaded->size != 4 * tex->source_tex->width * tex->source_tex->height)
					{
						KOSMOS_WARNING("Texture is " << tex->source_tex->width << "x" << tex->source_tex->height << " but bytes are " << loaded->size)
						empty_texture(tex->handle);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, tex->handle);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->source_tex->width, tex->source_tex->height,
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
			tex->dynamic = true;
			return tex;
		}

		texture_ref * load_texture(outki::texture *texture)
		{
			LoadedTextures::iterator i = s_textures.find(texture->id);
			if (i != s_textures.end())
			{
				i->second->refcount++;
				return i->second;
			}

			LIVE_UPDATE(&texture);

			if (!texture->output)
			{
				KOSMOS_WARNING("Trying to load a texture which has no generated output! [" << texture->id << "]");
				return 0;
			}

			texture_ref *tex = new texture_ref();
			tex->refcount = 1;
			tex->source = texture->id;
			tex->container = texture->output->data;
			tex->source_tex = texture;
			tex->dynamic = false;
			glGenTextures(1, &tex->handle);

			s_textures.insert(LoadedTextures::value_type(texture->id, tex));

			update_texture(tex);
			return tex;
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
					if (i->second->source_tex->output)
						i->second->container = i->second->source_tex->output->data;
					else
						i->second->container = 0;
					upd = true;
				}

				if (i->second->container)
				{
					if (LIVE_UPDATE(&i->second->container))
						upd = true;
					if (LIVE_UPDATE(&i->second->container->output))
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
			glClearColor(1, 1, 1, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
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

		int tex_id(texture_ref *tex)
		{
			return tex->handle;
		}
	}
}