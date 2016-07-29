#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/log.h>
#include <putki/builder/build-db.h>

#include <inki/types/kosmos/Shader.h>

#include <iostream>

namespace {

	bool build_shader(const putki::builder::build_info* info)
	{
		inki::shader* shader = (inki::shader* )info->object;

		if (!shader->data.empty())
		{
			if (!shader->source_file.empty())
			{
				RECORD_WARNING(info->record, "Ignoring SourceFile because there is data in the shader already");
			}
			return true;
		}

		putki::builder::resource res;
		if (!putki::builder::fetch_resource(info, shader->source_file.c_str(), &res))
		{
			RECORD_ERROR(info->record, "Failed to read [" << shader->source_file << "]!");
		}

		shader->data.insert(shader->data.begin(), (char*)res.data, (char*)res.data + res.size);
		RECORD_INFO(info->record, "Shader is " << shader->data.size() << " bytes");		
		putki::builder::free_resource(&res);
		return true;
	}

	bool build_shader_program(const putki::builder::build_info* info)
	{
		inki::shader_program *shader = (inki::shader_program *) info->object;

		if (!shader->vertex_text.data.empty())
		{
			if (shader->vertex_shader.get())
			{
				RECORD_WARNING(info->record, "Replaced vertex shader pointer with supplied text");
			}

			putki::ptr<inki::vertex_shader> shdr = putki::builder::create_build_output<inki::vertex_shader>(info, "vs");
			shdr->data.insert(shdr->data.begin(), shader->vertex_text.data.begin(), shader->vertex_text.data.end());	
			shader->vertex_shader = shdr;
		}

		if (!shader->fragment_text.data.empty())
		{
			if (shader->fragment_shader.get())
			{
				RECORD_WARNING(info->record, "Replaced vertex shader pointer with supplied text");
			}

			putki::ptr<inki::fragment_shader> shdr = putki::builder::create_build_output<inki::fragment_shader>(info, "fs");
			shdr->data.insert(shdr->data.begin(), shader->fragment_text.data.begin(), shader->fragment_text.data.end());
			shader->fragment_shader = shdr;
		}

		return true;
	}
};

void register_shader_builder(putki::builder::data *builder)
{
	putki::builder::handler_info info[3] = {
		{ inki::shader_program::type_id(), "prog-builder-1", build_shader_program, 0 },
		{ inki::vertex_shader::type_id(), "vs-builder-1", build_shader, 0 },
		{ inki::fragment_shader::type_id(), "fs-builder-1", build_shader, 0 },
	};
	putki::builder::add_handlers(builder, &info[0], &info[3]);
}
