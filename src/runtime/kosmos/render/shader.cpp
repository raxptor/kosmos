
#include <putki/liveupdate/liveupdate.h>
#include <outki/types/kosmos/Shader.h>

#include "log/log.h"
#include "shader.h"
#include "glwrap/gl.h"

#include <map>


namespace kosmos
{
	namespace shader
	{
		struct program
		{
			GLuint glprog;
		};
		
		typedef std::map<outki::Shader*, GLuint> ShaderMap;
		typedef std::map<outki::ShaderProgram*, GLuint> ShaderProgramMap;
		
		ShaderMap sh_cache;
		ShaderProgramMap prog_cache;
		
		bool get_compiled_shader(outki::Shader *shader, GLuint type, GLuint *out)
		{
			LIVE_UPDATE(&shader);
			ShaderMap::iterator i = sh_cache.find(shader);
			if (i != sh_cache.end())
			{
				if (LIVE_UPDATE(&shader->Data))
				{
					sh_cache.erase(i);
					return get_compiled_shader(shader, type, out);
				}

				*out = i->second;
				return true;
			}
				
			GLuint sh = glCreateShader(type);
			glShaderSource(sh, 1, &shader->Data, 0);
			glCompileShader(sh);
			
			GLint success;
			glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
			if (success == GL_FALSE)
			{
				KOSMOS_WARNING(shader->Data);
				KOSMOS_WARNING("Compilation of shader failed")
				return false;
			}
			else
			{
				KOSMOS_DEBUG("Compiled shader.");
			}
			
			sh_cache.insert(ShaderMap::value_type(shader, sh));
			*out = sh;
			return true;
		}
	
		bool get_program(outki::ShaderProgram *prog, GLuint *out)
		{
			LIVE_UPDATE(&prog);
			ShaderProgramMap::iterator i = prog_cache.find(prog);
			if (i != prog_cache.end())
			{
				*out = i->second;
				return true;
			}
			
			GLuint vsh, fsh;
			
			if (!get_compiled_shader(prog->VertexShader->up_cast<outki::Shader>(), GL_VERTEX_SHADER, &vsh))
			{
				KOSMOS_WARNING("Shader program has no valid vertex shader")
				return false;
			}

			if (!get_compiled_shader(prog->FragmentShader->up_cast<outki::Shader>(), GL_FRAGMENT_SHADER, &fsh))
			{
				KOSMOS_WARNING("Shader program has no valid vertex shader")
				return false;
			}
			
			GLuint prg = glCreateProgram();
			glAttachShader(prg, fsh);
			glAttachShader(prg, vsh);
			glLinkProgram(prg);
			
			GLint linked;
			glGetProgramiv(prg, GL_LINK_STATUS, &linked);
			if (linked != GL_TRUE)
			{
				KOSMOS_ERROR("Program linking failed!");
			}
			else
			{
				KOSMOS_INFO("Linked shader program " << prg);
			}
			
			prog_cache.insert(ShaderProgramMap::value_type(prog, prg));
			*out = prg;
			return true;
		}
		
		program *program_get(outki::ShaderProgram *prog)
		{
			GLuint p;
			if (get_program(prog, &p))
			{
				program *outp = new program();
				outp->glprog = p;
				return outp;
			}
			return 0;
		}
		
		void program_free(program *p)
		{
			delete p;
		}
		
		void program_use(program *p)
		{
			if (LIVEUPDATE_ISNULL(p))
				return;

			if (!p)
			{
				KOSMOS_ERROR("Program is null!");
			}

			glUseProgram(p->glprog);
		}
		
		int find_attribute(program *p, const char *name)
		{
			return glGetAttribLocation(p->glprog, name);
		}
		
		int find_uniform(program *p, const char *name)
		{
			return glGetUniformLocation(p->glprog, name);
		}
	}



}