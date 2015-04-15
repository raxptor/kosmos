import inki.Kosmos;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;
import putked.DataHelper;
import putked.Editor;
import putked.Interop.MemInstance;
import putked.Interop.Type;

public class TextureEditor implements Editor 
{
	@Override
	public String getName() 
	{
		return "TextureEditor";
	}

	@Override
	public boolean canEdit(Type type) 
	{
		return type.hasParent(Kosmos.Texture._getType());
	}

	@Override
	public Node createUI(MemInstance mi) 
	{
		putked.ProxyObject wrap = DataHelper.createPutkEdObj(mi);
		Kosmos.Texture tex = (Kosmos.Texture) wrap;
		if (tex == null)
			return null;
		
		putked.PropertyEditor pe = new putked.PropertyEditor();
		VBox box = new VBox();
		box.getChildren().add(new Label("Texture Editor [" + tex.getSource() + "]"));
		box.getChildren().add(pe.createUI(mi));
		return box;
	}
}
