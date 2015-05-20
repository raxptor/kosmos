import inki.Kosmos;
import javafx.application.Application;
import javafx.stage.Stage;
import putked.*;

public class Main extends Application 
{
	public static void main(String[] args) 
	{
		putked.Main.initEditor();
		initEditor();
		Main.launch(args);
	}
	
	public static void initEditor()
	{
		DataHelper.addTypeService(new Kosmos());
		putked.Main.addEditor(new TextureEditor());		
		putked.Main.addImporter(new TextureImporter());
	}
    
    @Override
    public void start(Stage stage) 
    {
    	putked.Main m = new putked.Main();
    	m.start(stage);
    }
}