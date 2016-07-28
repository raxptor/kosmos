#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/log.h>
#include <putki/builder/resource.h>
#include <putki/builder/build-db.h>
#include <putki/builder/db.h>

#include "kosmos-builder-utils/pngutil.h"
#include "kosmos-builder-utils/jpge.h"

#include <inki/types/kosmos/DataContainer.h>

#include <iostream>

namespace {
	const char *builder_version = "ksms-data-container";
}

struct databuilder : putki::builder::handler_i
{
	virtual const char *version() {
		return builder_version;
	}

	virtual bool handle(putki::builder::build_context *context, putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj)
	{
		inki::data_container *cont = (inki::data_container *) obj;
		inki::data_container_configuration *conf = cont->config;
		
		if (!conf)
		{
			RECORD_WARNING(record, "No configuration, clearing output to save space")
			cont->bytes.clear();
			return true;
		}
		
		putki::build_db::add_input_dependency(record, putki::db::pathof(input, conf));


		if (!cont->source_file.empty())
		{
			cont->bytes.clear();
			RECORD_INFO(record, "Using source file from " << cont->source_file);

			if (cont->file_type.empty())
			{
				// Steal file type from input
				int last_dot = cont->source_file.find_last_of('.');
				if (last_dot != std::string::npos)
					cont->file_type = cont->source_file.substr(last_dot + 1, cont->source_file.size() - last_dot - 1);
			}

			if (conf->mode != inki::DCOUT_DISCARD)
			{
				const char *bytes;
				long long size;
				if (!putki::resource::load(builder, cont->source_file.c_str(), &bytes, &size))
				{
					RECORD_ERROR(record, "Failed to read [" << cont->source_file << "]!");

				}
				else
				{
					cont->bytes.insert(cont->bytes.begin(), bytes, bytes + size);
					putki::resource::free(bytes);
				}
			}
		}
				
		switch (conf->mode)
		{
			case inki::DCOUT_FILE:
				{
					if (!conf->file_base)
					{
						RECORD_ERROR(record, "Mode is DCOUT_FILE, but missing file_base")
						return false;
					}
					
					putki::build_db::add_input_dependency(record, putki::db::pathof(input, conf->file_base));
				
					std::string filePath = conf->file_base->path_prefix;
					if (!filePath.empty() && filePath[filePath.size()-1] != '/')
						filePath.append("/");
					filePath.append(path);
					filePath.append(".");
					if (cont->file_type.empty())
						filePath.append("bin");
					else
						filePath.append(cont->file_type);
					RECORD_INFO(record, "Storing file to " << filePath)
					putki::resource::save_output(builder, filePath.c_str(), (const char*)&cont->bytes[0], cont->bytes.size());
					cont->bytes.clear();
					
					inki::data_container_output_file *file = inki::data_container_output_file::alloc();
					file->file_path= filePath;
					cont->output = file;
					
					std::string pth(path);
					add_output(context, record, (pth + "_tag").c_str(), file);
					break;
				}
				
			case inki::DCOUT_EMBED:
				RECORD_INFO(record, "Embedding file (" << cont->bytes.size() << ") bytes")
				break;
		
			case inki::DCOUT_DISCARD:
			default:
				RECORD_INFO(record, "Mode says to discard, producing empty object")
				cont->bytes.clear();
				break;
		}

		return false;
	}
};

void register_data_container_builder(putki::builder::data *builder)
{
	static databuilder fb;
	putki::builder::add_data_builder(builder, "DataContainer", &fb);
}
