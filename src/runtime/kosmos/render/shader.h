#ifndef __KOSMOS_SHADER_H__
#define __KOSMOS_SHADER_H__

namespace outki
{
	struct Shader;
	struct VertexShader;
	struct FragmentShader;
	struct ShaderProgram;
}

namespace kosmos
{
	namespace shader
	{
		struct program;
	
		program *program_get(outki::ShaderProgram *prog);
		void program_free(program *p);
		void program_use(program *p);
		
		int find_attribute(program *p, const char *name);
		int find_uniform(program *p, const char *name);
	}
}

#endif