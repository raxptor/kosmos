#ifndef __KOSMOS_SHADER_H__
#define __KOSMOS_SHADER_H__

namespace outki
{
	struct shader;
	struct vertex_shader;
	struct fragment_shader;
	struct shader_program;
}

namespace kosmos
{
	namespace shader
	{
		struct program;
	
		program *program_get(outki::shader_program *prog);
		void program_free(program *p);
		void program_use(program *p);
		
		int find_attribute(program *p, const char *name);
		int find_uniform(program *p, const char *name);
	}
}

#endif