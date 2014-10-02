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
		inki::Shader *shader = (inki::Shader *)obj;

		if (!shader->Data.empty())
		{
			if (!shader->SourceFile.empty())
			{
				RECORD_WARNING(record, "Ignoring SourceFile because there is data in the shader already");
			}
			return false;	
		}

		putki::build_db::add_external_resource_dependency(record, shader->SourceFile.c_str(), putki::resource::signature(builder, shader->SourceFile.c_str()).c_str());

		const char *bytes;
		long long size;
		if (!putki::resource::load(builder, shader->SourceFile.c_str(), &bytes, &size))
		{
			RECORD_ERROR(record, "Failed to read [" << shader->SourceFile << "]!");
		}

		shader->Data.insert(shader->Data.begin(), bytes, bytes + size);
		RECORD_INFO(record, "Shader is " << shader->Data.size() << " bytes");
		
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
		inki::ShaderProgram *shader = (inki::ShaderProgram *)obj;

		if (!shader->VertexText.data.empty())
		{
			if (shader->VertexShader)
				RECORD_WARNING(record, "Replaced vertex shader pointer with supplied text");

			inki::VertexShader *shdr = inki::VertexShader::alloc();
			shdr->parent.Data.insert(shdr->parent.Data.begin(), shader->VertexText.data.begin(), shader->VertexText.data.end());

			std::string pth(path);
			pth.append("_vert");

			shader->VertexShader = shdr;
			add_output(context, record, pth.c_str(), shdr);

		}

		if (!shader->FragmentText.data.empty())
		{
			if (shader->FragmentShader)
				RECORD_WARNING(record, "Replaced fragment shader pointer with supplied text");

			inki::FragmentShader *shdr = inki::FragmentShader::alloc();
			shdr->parent.Data.insert(shdr->parent.Data.begin(), shader->FragmentText.data.begin(), shader->FragmentText.data.end());

			std::string pth(path);
			pth.append("_frag");

			shader->FragmentShader = shdr;
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
