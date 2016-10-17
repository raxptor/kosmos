package kosmos;

import javafx.animation.AnimationTimer;
import javafx.geometry.Pos;
import putked.inki.Kosmos;
import putked.inki.Kosmos.*;
import javafx.scene.Node;
import javafx.scene.canvas.Canvas;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Label;
import javafx.scene.image.Image;
import javafx.scene.layout.Priority;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import putked.Editor;
import putked.Main;
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
		return type.hasParent(Main.s_compiler.getTypeByName("Texture"));
	}

	@Override
	public Node createUI(DataObject obj)
	{
		putked.ProxyObject wrap = putked.DataHelper.createPutkEdObj(obj);
		Kosmos.Texture tex = (Kosmos.Texture) wrap;
		if (tex == null)
			return null;

		VBox info = new VBox();
		info.setAlignment(Pos.CENTER);

		class _TMP {
			public int version;
		}

		final _TMP tmp = new _TMP();
		tmp.version = -1;

		AnimationTimer timer = new AnimationTimer() {
			@Override
			public void handle(long now) {
				int nv = obj.getVersion();
				if (tmp.version != nv) {
					tmp.version = nv;

					String source = Main.s_instance.translateResPathURI(tex.getSource());
					Image teximg = new Image(source, false);
					if (!teximg.isError())
					{
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
						info.getChildren().setAll(new Label("Failed to load!"));
					}
				}
			}
		};

		info.sceneProperty().addListener((p, oldValue, newValue) -> {
			if (oldValue == null && newValue != null) {
				timer.start();
			} else if (oldValue != null && newValue == null) {
				timer.stop();
			}
		});

		VBox box = new VBox();
		putked.PropertyEditor pe = new putked.PropertyEditor();
		Node proped = pe.createUI(obj);
		box.getChildren().add(proped);
		box.getChildren().add(info);
		VBox.setVgrow(info,  Priority.ALWAYS);

		info.getStyleClass().add("generic-custom-editor-box");

		return box;
	}
}
