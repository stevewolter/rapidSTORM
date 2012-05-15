package au.com.pulo.kev.simparm;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.text.NumberFormat;
import java.text.ParseException;
import java.io.*;
import java.util.LinkedList;

class NumberField 
implements MouseWheelListener, ActionListener, FocusListener
{
    protected JTextField text_field;
    protected NumberParser parser;
    protected NumberFieldListener listener;
    private String last_value = null;

    public static final int Value = 0;
    public static final int Increment = 1;
    public static final int Min = 2;
    public static final int Max = 3;

    boolean expect_event = false;

    public NumberField( NumberParser p, NumberFieldListener l ) {
        parser = p;
        listener = l;

        text_field = new JTextField();
        text_field.addActionListener(this);
        text_field.addMouseWheelListener(this);
        text_field.addFocusListener(this);
    }

    public void actionPerformed(ActionEvent e) { if ( ! expect_event) check_for_changes(); }
    public void focusGained(FocusEvent e) {} 
    public void focusLost(FocusEvent e) { if ( ! expect_event) check_for_changes(); } 

    private void check_for_changes() {
        if ( last_value == null || ! last_value.equals( text_field.getText() ) ) {
            last_value = text_field.getText();
            parser.set_string_value( last_value );
            listener.contents_changed( this );
        }
    }

    public String get_value(int type) {
        return parser.get_simparm_value(type); 
    }
    public void set_value_from_simparm(String s, int type) {
        String tf = parser.set_value_from_simparm( s, type );
        expect_event = true;
        text_field.setText( tf );
        expect_event = false;
    }
    public JComponent get_component() { return text_field; }
    public LinkedList<Component> get_components() { 
        LinkedList<Component> rv = new LinkedList<Component>();
        rv.add( text_field );
        return rv;
    }

    public void mouseWheelMoved(MouseWheelEvent e) {
        parser.increment_value(-e.getWheelRotation());
        listener.contents_changed( this );
    }

    public void set_availability( boolean is_available, int field ) {}
}
