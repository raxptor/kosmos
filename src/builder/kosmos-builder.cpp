#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/objstore.h>
#include <putki/builder/log.h>
#include <putki/builder/typereg.h>
#include <putki/builder/signature.h>
#include <putki/builder/package.h>

#include <inki/types/kosmos/DataContainer.h>

// generated.
namespace inki
{
	void bind_kosmos();
}

//
void register_texture_builder(putki::builder::data *builder);
void register_atlas_builder(putki::builder::data *builder);
void register_data_container_builder(putki::builder::data *builder);
void register_shader_builder(putki::builder::data *builder);

void kosmos_register_handlers(putki::builder::data *builder)
{
	register_texture_builder(builder);
	register_atlas_builder(builder);
	register_data_container_builder(builder);
	register_shader_builder(builder);
}

void kosmos_streamer_postbuild(putki::build::postbuild_info* info)
{
	const size_t max = 4096;
	const char* paths[max];
	size_t entries = putki::objstore::query_by_type(info->temp, inki::data_container_streaming_resource::th(), paths, max);
	if (entries > max)
	{
		APP_ERROR("Buffer too small.");
	}

	inki::data_container_streaming_info reg;
	
	putki::package::data *pkg = putki::build::create_package(info->pconf);
	putki::package::add(pkg, "streaming-info", false, true);

	for (size_t i = 0; i < entries; i++)
	{
		putki::objstore::object_info qi;
		if (putki::objstore::query_object(info->temp, paths[i], &qi))
		{
			putki::objstore::fetch_obj_result res;
			if (putki::objstore::fetch_object(info->temp, paths[i], &res))
			{
				std::string res_path = ((inki::data_container_streaming_resource*) res.obj)->resource_path;
				res.th->free(res.obj);
				putki::package::add_file(pkg, res_path.c_str(), true);
			}
			else
			{
				APP_ERROR("Could not fetch obj [" << paths[i] << "]!");
			}
		}
		else
		{
			APP_ERROR("Could not query obj [" << paths[i] << "]!");
		}
		putki::builder::add_build_root(info->builder, paths[i], 1);
	}


	for (size_t i = 0; i < entries; i++)
	{
		putki::package::add(pkg, paths[i], false, true);
	}

	reg.textures.reserve(entries);
	for (size_t i = 0; i < entries; i++)
	{
		reg.textures.push_back(paths[i]);
	}

	putki::builder::add_post_build_object(info->builder, inki::data_container_streaming_info::th(), &reg, "streaming-info");
	putki::build::commit_package(pkg, info->pconf, "streaming.pkg");
}

void kosmos_register_postbuild()
{
	putki::build::add_postbuild_fn(kosmos_streamer_postbuild);
}
