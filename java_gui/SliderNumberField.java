package au.com.pulo.kev.simparm;
import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.text.NumberFormat;
import java.text.ParseException;
import java.io.*;
import java.util.LinkedList;

class SliderNumberField
extends NumberField 
{
    private JPanel panel;
    private JSlider slider;
    private boolean has_min, has_max;

    private class SliderChanged implements ChangeListener {
        public void stateChanged(ChangeEvent e) {
            parser.set_percentage( slider.getValue() );
            listener.contents_changed( SliderNumberField.this );
        }
    }

    public SliderNumberField( NumberParser p, NumberFieldListener l ) {
        super(p,l);
        has_min = has_max = false;
        slider = new JSlider();
        slider.addChangeListener(new SliderChanged());
        panel = new JPanel();
        panel.setLayout(new BoxLayout(panel,BoxLayout.X_AXIS));
        panel.add(text_field);
        panel.add(slider);
    }
    public JComponent get_component() { return panel; }

    public void set_value_from_simparm(String s, int type) {
        super.set_value_from_simparm(s,type);
	if ( has_min && has_max && parser.has_percentage() ) {
            slider.setValue( parser.get_percentage() );
            slider.setVisible( true );
        } else
            slider.setVisible( false );
    }

    public void set_availability( boolean is_available, int field ) {
             if ( field == NumberField.Min ) has_min = is_available;
        else if ( field == NumberField.Max ) has_max = is_available;
        slider.setVisible( has_min && has_max && parser.has_percentage() );
    }

    public LinkedList<Component> get_components() { 
        LinkedList<Component> rv = super.get_components();
        rv.add( slider );
        return rv;
    }

/*
    private void stateChanged(ChangeEvent e) {
        if (e.getSource() == major_slider || e.getSource() == minor_slider) {
            slider_pair_to_value( major_slider, minor_slider );
        }
    }
        class SliderPair {
            int major, minor; 
            SliderPair(int _major, int _minor) { major = _major; minor = _minor; }
            SliderPair(JSlider _major, JSlider _minor) 
                { major = _major.getValue(); minor = _minor.getValue() 50; }
            boolean equals (SliderPair o) { 
                return o.major == major && o.minor == minor; 
            }
        }
        public SliderPair value_to_slider_pair(NumberType value) {
            if ( maxvalue == null || minvalue == null )
                return new SliderPair(major_slider, minor_slider);
            double range = maxvalue.doubleValue() - minvalue.doubleValue();
            if ( range <= 0 ) return new SliderPair(50,50);
            double percentage = (value.doubleValue() - minvalue.doubleValue())
                                    / range;
            int major = (int)Math.round(percentage * 100);
            int minor = (int)(((percentage*100) - major) * 100) + 50;
            return new SliderPair(major, minor);
        }
        abstract public NumberType slider_pair_to_value(SliderPair pair);
        private NumberType slider_pair_to_value(JSlider a, JSlider b)
            { return slider_pair_to_value( new SliderPair(a,b) ); }
*/

}
