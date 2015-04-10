import javafx.application.Application;
import javafx.stage.Stage;
import putked.*;

public class Main extends Application 
{
    public static void main(String[] args) 
    {
       Main.launch(args);
    }
    
    @Override
    public void start(Stage stage) 
    {
    	inki.Kosmos.register();
    	putked.Main m = new putked.Main();
    	m.start(stage);
    }
}