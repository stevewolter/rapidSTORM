package au.com.pulo.kev.simparm;

import javax.swing.JOptionPane;
import java.util.LinkedList;

class ConfigMessage extends ConfigObject {
    private String title, message, help_file, helpID;
    private int mtype;
    private LinkedList<String> answers = new LinkedList<String>();
    private boolean respond = false;

    protected boolean processAttribute(String name, String value) throws java.io.IOException
    {
        if ( name.equals("title") )
            title = value;
        else if ( name.equals("message") )
            message = value;
        else if ( name.equals("help_file") )
            help_file = value;
        else if ( name.equals("severity") ) {
            if ( value.equals("Critical") || value.equals("Error") )
                mtype = JOptionPane.ERROR_MESSAGE;
            else if ( value.equals("Question") )
                mtype = JOptionPane.QUESTION_MESSAGE;
            else if ( value.equals("Warning") )
                mtype = JOptionPane.WARNING_MESSAGE;
            else if ( value.equals("Info") || value.equals("Debug") )
                mtype = JOptionPane.INFORMATION_MESSAGE;
            else
                mtype = JOptionPane.PLAIN_MESSAGE;
        } else if ( name.equals("helpID") ) {
            helpID = value;
        } else if ( name.equals("options") ) {
            answers = new LinkedList<String>();
            if ( value.equals("JustOK") || value.equals("OKCancel") ) {
                answers.add("OK");
            } else if ( value.equals("YesNo") || value.equals("YesNoCancel") ) {
                answers.add("Yes");
                answers.add("No");
            } 
            if ( value.equals("OKCancel") || value.equals("YesNoCancel") )
                answers.add("Cancel");

            respond = ! value.equals("JustOK");
        } else if ( name.equals("response") )
            ;
        else
            return super.processAttribute(name,value);
        return true;
    }
    
    protected void declareDescriptedPanels(DeclarationInfo info) {
      if ( info != null ) {
        String[] options = new String[0];
        if ( helpID.equals("") ) answers.add("Help");
        options = answers.toArray(options);

        String response;
        while (true) {
            int rv = JOptionPane.showOptionDialog(
                null, message, title, JOptionPane.DEFAULT_OPTION, mtype, null, options, options[0]);
            if ( options[rv].equals("Yes") || options[rv].equals("OK") )
                response = "OKYes";
            else if ( options[rv].equals("Help") ) {
                HelpManager.getSingleton().forID( helpID );
                continue;
            } else
                response = options[rv];
            break;
        }

        if ( respond )
            print("response set " + response);
      }
    }

}
