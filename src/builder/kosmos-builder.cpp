#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>

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
