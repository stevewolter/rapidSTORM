import javax.swing.JOptionPane;
import java.io.IOException;
import java.io.File;

public class HelpManager {
    private static File manual = null;
    public static void openManual() {
        try {
            String[] args = { "hh", manual.getPath() };
            Runtime.getRuntime().exec(args);
        } catch (IOException e) {
            System.err.println(e.getMessage());
        }
    }

    public static void forID( int id ) {
        if ( id == 0 )
            JOptionPane.showMessageDialog(null,
              "No help is available for this item.");
        try {
        String[] args = 
            { "hh", "-mapid", Integer.toString(id), manual.getPath() };
        Runtime.getRuntime().exec( args );
        } catch (IOException e) {
            System.err.println(e.getMessage());
        }
    }

    public static void setManualFile(File file) {
        manual = file;
    }
}
