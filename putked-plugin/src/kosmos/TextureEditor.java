package kosmos;

import putked.inki.Kosmos;
import javafx.animation.AnimationTimer;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.layout.VBox;
import javafx.scene.layout.Priority;
import javafx.scene.paint.Color;
import putked.DataHelper;
import putked.Editor;
import putked.DataObject;;

public class TextureEditor implements Editor
{
	@Override
	public String getName()
	{
		return "TextureEditor";
	}

	@Override
	public int getPriority()
	{
		return -1;
	}

	@Override
	public boolean canEdit(putki.Compiler.ParsedStruct type)
	{
		return false;
//		return type.hasParent(putked.inki.KOsmo)._getType());
	}

	@Override
	public Node createUI(DataObject obj)
	{
		/*
		putked.ProxyObject wrap = DataHelper.createPutkEdObj(mi);
		Kosmos.Texture tex = (Kosmos.Texture) wrap;
		if (tex == null)
			return null;

		VBox info = new VBox();
		info.setAlignment(Pos.CENTER);

		class _TMP {
			public int version;
		}

		final _TMP tmp = new _TMP();
		tmp.version = mi.getVersion();

		AnimationTimer at = new AnimationTimer() {
			@Override
			public void handle(long now) {
				int nv = mi.getVersion();
				if (tmp.version != nv) {
					tmp.version = nv;
					java.io.File f = new java.io.File(Interop.translateResPath(tex.getSource()));
					if (f.exists())
					{
						Image teximg = new Image("file:" + f.getAbsolutePath(), false);
						Canvas canv = new Canvas(teximg.getWidth(), teximg.getHeight());
						GraphicsContext gc = canv.getGraphicsContext2D();

						gc.setFill(Color.BLUE);
						gc.drawImage(teximg, 0, 0);

						Label label = new Label();
						label.setText((int)teximg.getWidth() + "x" + (int)teximg.getHeight());

						info.getChildren().setAll(canv, label);
					}
					else
					{
						info.getChildren().setAll(new Label("File does not exist!"));
					}
				}
			}
		};

		info.sceneProperty().addListener((p, oldValue, newValue) -> {
			if (oldValue == null && newValue != null) {
				at.start();
			} else if (oldValue != null && newValue == null) {
				at.stop();
			}
		});

		VBox box = new VBox();
		putked.PropertyEditor pe = new putked.PropertyEditor();
		Node proped = pe.createUI(mi);
		box.getChildren().add(proped);
		box.getChildren().add(info);
		VBox.setVgrow(info,  Priority.ALWAYS);

		info.getStyleClass().add("generic-custom-editor-box");

		return box;
		*/
		return null;
	}
}
