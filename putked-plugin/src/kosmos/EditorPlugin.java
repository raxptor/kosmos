package kosmos;

import putked.*;

public class EditorPlugin implements putked.EditorPluginDescription
{
	@Override
	public String getName() { return "KosmosPlugin"; }

	@Override
	public String getVersion() { return "1.0"; }

	@Override
	public PluginType getType() { return PluginType.PLUGIN_EDITOR; }

	@Override
	public void start()
	{
		DataHelper.addTypeService(new putked.inki.Kosmos());
		putked.Main.addEditor(new kosmos.TextureEditor());
		putked.Main.addImporter(new kosmos.TextureImporter());
	}
}
