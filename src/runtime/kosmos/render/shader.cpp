#include "shader.h"

#include <outki/types/kosmos/Shader.h>
#include "log/log.h"

#include <map>

#include <OpenGL/gl.h>


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
			ShaderMap::iterator i = sh_cache.find(shader);
			if (i != sh_cache.end())
			{
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
				KOSMOS_ERROR("Compilation of shader failed")
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
			ShaderProgramMap::iterator i = prog_cache.find(prog);
			if (i != prog_cache.end())
			{
				*out = i->second;
				return true;
			}
			
			GLuint vsh, fsh;
			
			if (!get_compiled_shader(prog->VertexShader->up_cast<outki::Shader>(), GL_VERTEX_SHADER, &vsh))
				KOSMOS_ERROR("Shader program has no valid vertex shader")
			if (!get_compiled_shader(prog->FragmentShader->up_cast<outki::Shader>(), GL_FRAGMENT_SHADER, &fsh))
				KOSMOS_ERROR("Shader program has no valid vertex shader")
			
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