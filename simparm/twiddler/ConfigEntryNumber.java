
// SimParm: Simple and flexible C++ configuration framework
// Copyright (C) 2007 Australian National University
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 
// Contact:
// Kevin Pulo
// kevin.pulo@anu.edu.au
// Leonard Huxley Bldg 56
// Australian National University, ACT, 0200, Australia

//package simparm.twiddler;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.text.NumberFormat;
import java.text.ParseException;
import java.io.*;

abstract class ConfigEntryNumber<NumberType extends Number>
    extends ConfigEntry 
    implements MouseWheelListener, ChangeListener
{
        private JPanel panel;
        protected JTextField textField;
        private JSlider major_slider, minor_slider;
        private boolean blockEvent;

	protected NumberType value;
	protected NumberType increment;
	protected boolean hasmin;
	protected NumberType minvalue;
	protected boolean hasmax;
	protected NumberType maxvalue;

        private boolean waiting_for_application_answer = false;
        private NumberType enqueued_change = null;
        private class NumberAcknowledgement extends Acknowledgement {
            NumberAcknowledgement() {
                waiting_for_application_answer = true;
            }
            public void gotAcknowledgement() {
                waiting_for_application_answer = false;
                if ( enqueued_change != null ) {
                    NumberType c = enqueued_change;
                    enqueued_change = null;
                    setValue( c, true, false, false );
                }
            }
        };

        public ConfigEntryNumber() {
            super();
            textField = new JTextField();
            textField.addActionListener(this);
            major_slider = new JSlider();
            major_slider.addChangeListener(this);
            // minor_slider = new JSlider();
            // minor_slider.addChangeListener(this);
            setField(textField);
            makeField();

            increment = null;
            setNoMin();
            setNoMax();
        }

        protected void makeField() {
            blockEvent = true;
            if (hasMinAndMax()) {
                if (panel == null) {
                    panel = new JPanel();
                    panel.setLayout(new BoxLayout(panel,BoxLayout.X_AXIS));
                    panel.add(textField);
                    panel.add(major_slider);
                    // panel.add(minor_slider);
                    setField(panel);
                    textField.setVisible(true);
                    major_slider.setVisible(true);
                    // minor_slider.setVisible(true);
                } else {
                    setValue( value, false, false, true );
                }
            } else if ( (! hasMinAndMax() ) && panel != null ) {
                panel = null;
                setField(textField);
            }
            blockEvent = false;
        }

	public void setLabel(JComponent label) {
		super.setLabel(label);
		label.addMouseWheelListener(this);
	}

	public void setField(JComponent field) {
		super.setField(field);
                if (field != null) {
                    field.addMouseWheelListener(this);
                    if ( field instanceof JTextField)
                        ((JTextField)field).addActionListener(this);
                }
	}

        public void stateChanged(ChangeEvent e) {
            if (blockEvent) return;
            if (e.getSource() == major_slider || e.getSource() == minor_slider) {
                NumberType sliderVal = 
                    slider_pair_to_value( major_slider, minor_slider );
                setValue( sliderVal, true, true, false );
            }
        }

	public void mouseWheelMoved(MouseWheelEvent e) {
            if (blockEvent) return;
            NumberType newVal = 
                modifyByIncrements(value, e.getWheelRotation());
            setValue( newVal, true, true, true );
	}

        private void setValueFromText() {
            if ( textField == null ) return;
            String text = textField.getText();
            try {
                NumberType dv = parseGUIValue(text);
                setValue(dv, true, false, true);
            } catch (ParseException ex) {
                System.err.println(
                    "Error in setting value from string '" + text + 
                    "' in ConfigEntryNumber '" + getName() + "': " + ex);
            }
        }

	public void commitChanges() {
            if (!blockEvent) setValueFromText();
	}

	public void setValue(NumberType newvalue, 
            boolean doprint,
            boolean updatefield,
            boolean updateslider
        ) {
		if (hasmin && 
                    newvalue.doubleValue() < minvalue.doubleValue()) 
                {
			newvalue = minvalue;
                        updatefield = true;
		}
		if (hasmax && 
                    newvalue.doubleValue() > maxvalue.doubleValue()) 
                {
			newvalue = maxvalue;
                        updatefield = true;
		}

		if (! newvalue.equals(value) ) {
			value = newvalue;
			if (doprint) {
                            if ( !waiting_for_application_answer ) {
                                waiting_for_application_answer = true;
				doPrint( new NumberAcknowledgement() );
                            } else {
                                enqueued_change = newvalue;
                            }
			}
		}
		if (updatefield) {
                    String gui_text = printGUIValue( newvalue );
                    blockEvent = true;
                    textField.setText(gui_text);
                    blockEvent = false;
                    validate();
		}
                if (updateslider) {
                    if ( major_slider != null &&
                         slider_pair_to_value( major_slider,
                            minor_slider ) != value ) 
                    {
                        SliderPair new_setting = 
                            value_to_slider_pair( value );
                        blockEvent = true;
                        major_slider.setValue(new_setting.major);
                        blockEvent = false;
                    }
                }
	}

	public boolean processCommand(String line, BufferedReader rest)
            throws IOException
        {
            String[] split = line.split(" ", 2);
            String command = split[0];
            boolean hasArg = (split.length == 2);
            String arg = null;
            if (hasArg) arg = split[1];

            boolean couldHandle = true;
            try {
                if (hasArg && command.equals("value")) {
                    if ( enqueued_change == null )
                        setValue( parseSimparmValue( arg ),
                                false, true, true );
                } else if (hasArg && command.equals("increment"))
                    setIncrement( parseSimparmValue(arg));
                else if (hasArg && command.equals("min")) {
                    minvalue = parseSimparmValue(arg);
                    makeField();
                } else if (hasArg && command.equals("max")) {
                    maxvalue = parseSimparmValue(arg);
                    makeField();
                } else
                    couldHandle = false;
            } catch (ParseException e) {
                throw new IOException("Unable to parse value: " + e.getMessage());
            }
            if (couldHandle)
                ;
            else if (hasArg && command.equals("has_min"))
                hasmin = Boolean.valueOf(arg);
            else if (hasArg && command.equals("has_max"))
                hasmax = Boolean.valueOf(arg);
            else
                return super.processCommand(line, rest);

            makeField();
            return true;
        }

        protected abstract String printGUIValue(NumberType e);
        protected abstract String printSimparmValue(NumberType e);
        protected abstract NumberType parseSimparmValue(String s)
            throws ParseException;
        protected abstract NumberType parseGUIValue(String s)
            throws ParseException;

	public void setMin(NumberType min) {
		hasmin = true;
		minvalue = min;
                makeField();
	}

	public void setNoMin() {
		hasmin = false;
                makeField();
	}

	public void setMax(NumberType max) {
		hasmax = true;
		maxvalue = max;
                makeField();
	}

	public void setNoMax() {
		hasmax = false;
                makeField();
	}

        public boolean hasMinAndMax() { 
            return hasmax && hasmin && 
                   minvalue != null && maxvalue != null; 
        }

        class SliderPair {
            int major, minor; 
            SliderPair(int _major, int _minor) { major = _major; minor = _minor; }
            SliderPair(JSlider _major, JSlider _minor) 
                { major = _major.getValue(); minor = /*_minor.getValue()*/ 50; }
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

	public void setIncrement(NumberType increment) {
		this.increment = increment;
	}

        protected String getStringValue() 
            { return printSimparmValue(value); }

        protected abstract NumberType 
            modifyByIncrements(NumberType x, int in);
}

