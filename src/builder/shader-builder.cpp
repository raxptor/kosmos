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

void register_shader_builder(putki::builder::data *builder)
{
	static vertexshader fb;
	putki::builder::add_data_builder(builder, "VertexShader", &fb);
	putki::builder::add_data_builder(builder, "FragmentShader", &fb);
}
