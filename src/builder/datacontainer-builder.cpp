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
	const char *builder_version = "ksms-datacontainer";
}

struct databuilder : putki::builder::handler_i
{
	virtual const char *version() {
		return builder_version;
	}

	virtual bool handle(putki::builder::build_context *context, putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj)
	{
		inki::DataContainer *cont = (inki::DataContainer *) obj;
		inki::DataContainerConfiguration *conf = cont->Config;
		
		if (!conf)
		{
			RECORD_WARNING(record, "No configuration, clearing output to save space")
			cont->Bytes.clear();
			return true;
		}
		
		putki::build_db::add_input_dependency(record, putki::db::pathof(input, conf));


		if (!cont->SourceFile.empty())
		{
			cont->Bytes.clear();
			RECORD_INFO(record, "Using source file from " << cont->SourceFile);

			if (conf->Mode != inki::DCOUT_FILE && conf->Mode != inki::DCOUT_DISCARD)
			{
				const char *bytes;
				long long size;
				if (!putki::resource::load(builder, cont->SourceFile.c_str(), &bytes, &size))
				{
					RECORD_ERROR(record, "Failed to read [" << cont->SourceFile << "]!");

				}
				else
				{
					cont->Bytes.insert(cont->Bytes.begin(), bytes, bytes + size);
					putki::resource::free(bytes);
				}
			}
		}


				
		switch (conf->Mode)
		{
			case inki::DCOUT_FILE:
				{
					if (!conf->FileBase)
					{
						RECORD_ERROR(record, "Mode is DCOUT_FILE, but missing FileBase")
						return false;
					}
					
					putki::build_db::add_input_dependency(record, putki::db::pathof(input, conf->FileBase));
				
					std::string filePath = conf->FileBase->PathPrefix;
					if (!filePath.empty() && filePath[filePath.size()-1] != '/')
						filePath.append("/");
					filePath.append(path);
					filePath.append(".");
					if (cont->FileType.empty())
						filePath.append("bin");
					else
						filePath.append(cont->FileType);
					RECORD_INFO(record, "Storing file to " << filePath)
					putki::resource::save_output(builder, filePath.c_str(), (const char*)&cont->Bytes[0], cont->Bytes.size());
					cont->Bytes.clear();
					
					inki::DataContainerOutputFile *file = inki::DataContainerOutputFile::alloc();
					file->FilePath = filePath;
					cont->Output = &file->parent;
					
					std::string pth(path);
					add_output(context, record, (pth + "_tag").c_str(), file);
					break;
				}
				
			case inki::DCOUT_EMBED:
				RECORD_INFO(record, "Embedding file (" << cont->Bytes.size() << ") bytes")
				break;
		
			case inki::DCOUT_DISCARD:
			default:
				RECORD_INFO(record, "Mode says to discard, producing empty object")
				cont->Bytes.clear();
				break;
		}

		return false;
	}
};

void register_datacontainer_builder(putki::builder::data *builder)
{
	static databuilder fb;
	putki::builder::add_data_builder(builder, "DataContainer", &fb);
}
