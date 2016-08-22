#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/log.h>
#include <putki/builder/build-db.h>

#include "kosmos-builder-utils/pngutil.h"
#include "kosmos-builder-utils/jpge.h"

#include <inki/types/kosmos/DataContainer.h>

#include <iostream>

namespace
{
	bool build_data_container(const putki::builder::build_info* info)
	{
		inki::data_container *cont = (inki::data_container *) info->object;
		putki::ptr<const inki::data_container_configuration> conf = cont->config;

		if (!conf)
		{
			RECORD_WARNING(info->record, "No configuration, clearing output to save space")
			cont->bytes.clear();
			return true;
		}

		if (!cont->source_file.empty())
		{
			cont->bytes.clear();
			RECORD_INFO(info->record, "Using source file from " << cont->source_file);

			if (cont->file_type.empty())
			{
				// Steal file type from input
				int last_dot = cont->source_file.find_last_of('.');
				if (last_dot != std::string::npos)
					cont->file_type = cont->source_file.substr(last_dot + 1, cont->source_file.size() - last_dot - 1);
			}

			if (conf->mode != inki::DCOUT_DISCARD)
			{
				putki::builder::resource res;
				if (!putki::builder::fetch_resource(info, cont->source_file.c_str(), &res))
				{
					RECORD_ERROR(info->record, "Failed to read [" << cont->source_file << "]!");
				}
				else
				{
					cont->bytes.insert(cont->bytes.begin(), res.data, res.data + res.size);
					putki::builder::free_resource(&res);
				}
			}
		}

		switch (conf->mode)
		{
			case inki::DCOUT_EMBED:
			{
				RECORD_INFO(info->record, "Embedding file (" << cont->bytes.size() << ") bytes")
				break;
			}
			case inki::DCOUT_STREAMING:
			{
				typedef inki::data_container_streaming_resource out;
				putki::ptr<out> o = putki::builder::create_build_output<out>(info, "sout");
				o->resource_path = putki::builder::store_resource_tag(info, "streaming", (const char*)&cont->bytes[0], cont->bytes.size());
				cont->streaming = o;
				
				RECORD_INFO(info->record, "Streaming file (" << cont->bytes.size() << ") bytes, file=" << o->resource_path);
				break;
			}
			case inki::DCOUT_DISCARD:
			default:
			{
				RECORD_INFO(info->record, "Mode says to discard, producing empty object")
				cont->bytes.clear();
				break;
			}
		}

		return true;
	}
};

void register_data_container_builder(putki::builder::data *builder)
{
	putki::builder::handler_info info[1] = {
		{ inki::data_container::type_id(), "datacontainer-builder-4", build_data_container, 0 }
	};
	putki::builder::add_handlers(builder, &info[0], &info[1]);
}
