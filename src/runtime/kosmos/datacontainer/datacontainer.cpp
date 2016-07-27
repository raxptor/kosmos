#include <putki/config.h>
#include <putki/log/log.h>

#include <putki/liveupdate/liveupdate.h>

#include "datacontainer/datacontainer.h"
#include "log/log.h"

#include <iostream>
#include <map>
#include <fstream>

namespace kosmos
{
	namespace datacontainer
	{
		struct loaded_data_internal : loaded_data
		{
			outki::data_container *source;
			bool allocated;
			int refcount;
		};

		typedef std::map<outki::data_container*, loaded_data_internal*> LoadMap;
		LoadMap s_loaded;

		loaded_data* load(outki::data_container *container, bool block_until_loaded)
		{
			LoadMap::iterator i = s_loaded.find(container);
			if (i != s_loaded.end())
			{
				if (LIVE_UPDATE(&i->second->source) || (i->second->source->output && LIVE_UPDATE(&i->second->source->output)))
				{
					if (--i->second->refcount == 0)
					{
						KOSMOS_DEBUG("Datacontainer updated out of existence")
						if (i->second->allocated)
							delete [] i->second->data;
					}
					s_loaded.erase(i);
				}
				else
				{
					KOSMOS_DEBUG("Had this loaded already, returning pointer to old record")
					i->second->refcount++;
					return i->second;
				}
			}

			loaded_data_internal *nr = new loaded_data_internal();
			nr->allocated = false;
			nr->refcount = 1;
			nr->source = container;

			outki::data_container_output *output = container->output;
			KOSMOS_DEBUG("Loading data container with output " << output);

			if (!output)
			{
				KOSMOS_DEBUG("Data was embedded");
				nr->data = container->bytes;
				nr->size = (size_t) container->bytes_size;
				s_loaded.insert(LoadMap::value_type(container, nr));
				return nr;
			}
			else if (outki::data_container_output_file *file = output->exact_cast<outki::data_container_output_file >())
			{
				char real_path[512];
				putki::format_file_path(file->file_path, real_path);
				KOSMOS_DEBUG("Loading file [" << real_path << "]");

				std::ifstream in(real_path, std::ios::binary);
				if (!in.good())
				{
					KOSMOS_DEBUG("Failed to open file [" << real_path << "]!");
					delete nr;
					return 0;
				}

				in.seekg(0, std::ios::end);
				std::streamoff filesize = in.tellg();

				nr->data = new unsigned char[(unsigned long)filesize];
				nr->size = (size_t) filesize;
				nr->allocated = true;
				in.seekg(0, std::ios_base::beg);
				in.read((char*)nr->data, filesize);
				in.close();
				s_loaded.insert(LoadMap::value_type(container, nr));
				return nr;
			}
			else
			{
				KOSMOS_DEBUG("Data was not embedded");
				return 0;
			}
		}

		void release(loaded_data *d)
		{
			loaded_data_internal *record = (loaded_data_internal*) d;
			--record->refcount;
			KOSMOS_DEBUG("Refcount is now " << record->refcount);
			if (!record->refcount)
			{
				s_loaded.erase(s_loaded.find(record->source));
				delete record;
			}
		}
	}
}