#include <putki/config.h>
#include <putki/log/log.h>

#include <putki/liveupdate/liveupdate.h>
#include <putki/pkgmgr.h>
#include <putki/pkgloader.h>

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
			putki::pkgmgr::resource_data resource;
			bool is_loaded_resource;
			outki::data_container *source;
			int refcount;
		};

		typedef std::map<outki::data_container*, loaded_data_internal*> LoadMap;
		LoadMap s_loaded;
		putki::pkgmgr::loaded_package* s_stream_pkg = 0;

		void init()
		{
			s_stream_pkg = putki::pkgloader::from_file("streaming.pkg");
			if (!s_stream_pkg)
			{
				KOSMOS_WARNING("Failed to load streaming package!");
			}
		}

		void destroy()
		{
			if (s_stream_pkg)
			{
				putki::pkgmgr::release(s_stream_pkg);
				s_stream_pkg = 0;
			}
		}

		loaded_data* load(outki::data_container *container, bool block_until_loaded)
		{
			LoadMap::iterator i = s_loaded.find(container);
			if (i != s_loaded.end())
			{
				if (LIVE_UPDATE(&i->second->source) || (i->second->source->streaming && LIVE_UPDATE(&i->second->source->streaming)))
				{
					if (--i->second->refcount == 0)
					{
						KOSMOS_DEBUG("Datacontainer updated out of existence")
						if (i->second->is_loaded_resource)
						{
							putki::pkgmgr::free_resource(&i->second->resource);
							delete[] i->second->data;
						}
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
			nr->is_loaded_resource = false;
			nr->refcount = 1;
			nr->source = container;

			outki::data_container_streaming_resource *streaming = container->streaming;
			KOSMOS_DEBUG("Loading data container with output " << streaming);

			bool load_result;
			if (streaming)
			{
				KOSMOS_DEBUG("Attempting load from streaming package, path [" << container->streaming->streaming_file << "]");
				load_result = putki::pkgmgr::load_resource(s_stream_pkg, container->streaming->streaming_file, &nr->resource);
			}
			else
			{
				KOSMOS_DEBUG("Attempting load embedded resource with streaming as backup..")
				load_result = putki::pkgmgr::load_resource(container->embedded_file, &nr->resource, s_stream_pkg);
			}

			if (!load_result)
			{
				KOSMOS_ERROR("Failed to load resource!");
				nr->data = 0;
				nr->size = 0;
			}
			else
			{
				nr->data = nr->resource.data;
				nr->size = nr->resource.size;
				nr->is_loaded_resource = true;
			}
			s_loaded.insert(LoadMap::value_type(container, nr));
			return nr;
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