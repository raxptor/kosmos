#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/log.h>
#include <putki/builder/resource.h>
#include <putki/builder/build-db.h>
#include <putki/builder/db.h>

#include <inki/types/kosmos/Shader.h>

#include <iostream>

namespace {
	const char *builder_version = "shaderbuilder-2";
}

struct vertexshader : putki::builder::handler_i
{
	virtual const char *version() {
		return builder_version;
	}

	virtual bool handle(putki::builder::build_context *context, putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj)
	{
		inki::shader *shader = (inki::shader *)obj;

		if (!shader->data.empty())
		{
			if (!shader->source_file.empty())
			{
				RECORD_WARNING(record, "Ignoring SourceFile because there is data in the shader already");
			}
			return false;	
		}

		putki::build_db::add_external_resource_dependency(record, shader->source_file.c_str(), putki::resource::signature(builder, shader->source_file.c_str()).c_str());

		const char *bytes;
		long long size;
		if (!putki::resource::load(builder, shader->source_file.c_str(), &bytes, &size))
		{
			RECORD_ERROR(record, "Failed to read [" << shader->source_file << "]!");
		}

		shader->data.insert(shader->data.begin(), bytes, bytes + size);
		RECORD_INFO(record, "Shader is " << shader->data.size() << " bytes");
		
		putki::resource::free(bytes);
		return false;
	}
};

struct shaderprogram : putki::builder::handler_i
{
	virtual const char *version() {
		return builder_version;
	}

	virtual bool handle(putki::builder::build_context *context, putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj)
	{
		inki::shader_program *shader = (inki::shader_program *)obj;

		if (!shader->vertex_text.data.empty())
		{
			if (shader->vertex_shader)
				RECORD_WARNING(record, "Replaced vertex shader pointer with supplied text");

			inki::vertex_shader *shdr = inki::vertex_shader::alloc();
			shdr->data.insert(shdr->data.begin(), shader->vertex_text.data.begin(), shader->vertex_text.data.end());

			std::string pth(path);
			pth.append("_vert");

			shader->vertex_shader = shdr;
			add_output(context, record, pth.c_str(), shdr);

		}

		if (!shader->fragment_text.data.empty())
		{
			if (shader->fragment_shader)
				RECORD_WARNING(record, "Replaced fragment shader pointer with supplied text");

			inki::fragment_shader *shdr = inki::fragment_shader::alloc();
			shdr->data.insert(shdr->data.begin(), shader->fragment_text.data.begin(), shader->fragment_text.data.end());

			std::string pth(path);
			pth.append("_frag");

			shader->fragment_shader = shdr;
			add_output(context, record, pth.c_str(), shdr);
		}

		return false;
	}
};

void register_shader_builder(putki::builder::data *builder)
{
	static vertexshader fb;
	static shaderprogram sp;
	putki::builder::add_data_builder(builder, "VertexShader", &fb);
	putki::builder::add_data_builder(builder, "FragmentShader", &fb);
	putki::builder::add_data_builder(builder, "ShaderProgram", &sp);
}
