#ifndef __KOSMOS_DATACONTAINER_H__
#define __KOSMOS_DATACONTAINER_H__

#include <outki/types/kosmos/DataContainer.h>
#include <stddef.h>

namespace datacontainer
{
	struct loaded_data
	{
		unsigned char *data;
		size_t size;
	};

	loaded_data* load(outki::DataContainer *container, bool block_until_loaded);
	void release(loaded_data *d);
}

#endif