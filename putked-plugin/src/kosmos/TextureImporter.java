package kosmos;

import inki.Kosmos;

import java.io.*;

import javafx.stage.FileChooser;
import javafx.stage.FileChooser.ExtensionFilter;
import putked.DataHelper;
import putked.DataImporter;
import putked.Interop;

public class TextureImporter implements DataImporter 
{
	public String getName()
	{
		return "Texture";
	}
	
	public boolean importTo(String path)
	{
		System.out.println("Importing to [" + path +"]");
		FileChooser fileChooser = new FileChooser();
		fileChooser.setTitle("Open Resource File");
		fileChooser.getExtensionFilters().addAll(new ExtensionFilter("PNG Files", "*.png"));
		File selectedFile = fileChooser.showOpenDialog(null);
		if (selectedFile != null) {

			putked.Main.ImportFinalizationQuestion q = new putked.Main.ImportFinalizationQuestion();
			String name = selectedFile.getName();
			int p = name.indexOf('.');
			if (p >= 0)
				q.proposedPath = path + name.substring(0, p);
			else
				q.proposedPath = path + name;
			
			q.proposedResPath = path + name;
			putked.Main.s_instance.askImportFinalization(q, null);
			
			if (q.accepted)
			{
				try {
					Interop.writeResFile(selectedFile, q.proposedResPath);
					Interop.Type t = Kosmos.Texture._getType();
					Kosmos.Texture tex = (Kosmos.Texture) DataHelper.createPutkEdObj(t.createInstance(q.proposedPath));
					tex.setSource(q.proposedResPath);
					tex.m_mi.diskSave();
					System.out.println("Import successful");
					putked.Main.s_instance.startEditing(tex.m_mi.getPath());
					return true;
				} catch (IOException e) {
					System.out.println("Failed to write res file!");
					return false;
				}
			}
		}
		return false;
	}
}
