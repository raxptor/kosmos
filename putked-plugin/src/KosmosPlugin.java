import inki.Kosmos;
import putked.*;

public class KosmosPlugin implements putked.EditorPluginDescription
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
		DataHelper.addTypeService(new Kosmos());
		putked.Main.addEditor(new TextureEditor());		
		putked.Main.addImporter(new TextureImporter());
	}
}
