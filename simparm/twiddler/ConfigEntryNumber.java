
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
    implements MouseWheelListener, ActionListener, ChangeListener
{
        private JPanel panel;
        protected JTextField textField;
        private JSlider slider;
        private boolean blockEvent;

	protected NumberType value;
	protected NumberType increment;
	protected boolean hasmin;
	protected NumberType minvalue;
	protected boolean hasmax;
	protected NumberType maxvalue;

        public ConfigEntryNumber() {
            super();
            textField = new JTextField();
            textField.addActionListener(this);
            slider = new JSlider();
            slider.addChangeListener(this);
            setField(textField);
            makeField();

            increment = null;
            setNoMin();
            setNoMax();
        }

        protected void makeField() {
            blockEvent = true;
            if (hasMinAndMax()) {
                slider.setValue(getPercentageValue());
                if (panel == null) {
                    panel = new JPanel();
                    panel.setLayout(new BoxLayout(panel,BoxLayout.X_AXIS));
                    panel.add(textField);
                    panel.add(slider);
                    setField(panel);
                    textField.setVisible(true);
                    slider.setVisible(true);
                }
            } else if ( (! hasMinAndMax() ) && panel != null ) {
                panel = null;
                setField(textField);
            }
            blockEvent = false;
        }

        protected void setField(String text) {
            blockEvent = true;
            if (textField != null)
                textField.setText(text);
            if (hasMinAndMax())
                slider.setValue(getPercentageValue());
            blockEvent = false;
        }

	public void setLabel(JComponent label) {
		super.setLabel(label);
		this.label.addMouseWheelListener(this);
	}

	public void setField(JComponent field) {
		super.setField(field);
		this.field.addMouseWheelListener(this);
	}

        public void stateChanged(ChangeEvent e) {
            if (blockEvent) return;
            if (e.getSource() == slider) {
                setPercentageValue( slider.getValue() );
            }
        }

	public void mouseWheelMoved(MouseWheelEvent e) {
            if (blockEvent) return;
		adjustValue(e.getWheelRotation());
	}

        private void setValueFromText() {
            try {
                String text = textField.getText();
                NumberType dv = parseGUIValue(text);
                setValue(dv, true, false);
            } catch (ParseException ex) {
                System.err.println(ex);
            }
        }

	public void actionPerformed(ActionEvent e) {
            if (!blockEvent) setValueFromText();
	}

        public void mouseExited(MouseEvent e) {
            super.mouseExited(e);
            if (!blockEvent) setValueFromText();
        }

	public boolean setValue(String value, boolean doprint, 
                                boolean updatefield) 
        {
            try {
                setValue( parseSimparmValue(value), doprint, updatefield);
                return true;
            } catch (NumberFormatException ex) {
                System.err.println(ex);
                return false;
            } catch (ParseException ex) {
                System.err.println(ex);
                return false;
            }
	}

	public void setValue(NumberType newvalue, boolean doprint,
                             boolean updatefield) 
        {
		if (hasmin && 
                    newvalue.doubleValue() < minvalue.doubleValue()) 
                {
			newvalue = minvalue;
		}
		if (hasmax && 
                    newvalue.doubleValue() > maxvalue.doubleValue()) 
                {
			newvalue = maxvalue;
		}

		if (! newvalue.equals(value) ) {
			value = newvalue;
			if (doprint) {
				doPrint();
			}
		}
		if (updatefield) {
                    textField.setText(printGUIValue( newvalue ));
		}
	}

        public void setValue(NumberType newvalue)
            { setValue(newvalue, false, true); }

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
                if (hasArg && command.equals("value"))
                        setValue( parseSimparmValue( arg ) );
                else if (hasArg && command.equals("increment"))
                    setIncrement( parseSimparmValue(arg));
                else if (hasArg && command.equals("min"))
                    setMin( parseSimparmValue(arg) );
                else if (hasArg && command.equals("max"))
                    setMax( parseSimparmValue(arg) );
                else
                    couldHandle = false;
            } catch (ParseException e) {
                throw new IOException("Unable to parse value: " + e.getMessage());
            }
            if (couldHandle)
                ;
            else if (hasArg && command.equals("hasmin"))
                hasmin = Boolean.valueOf(arg);
            else if (hasArg && command.equals("hasmax"))
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

        public boolean hasMinAndMax() { return hasmax && hasmin; }

        abstract public int getPercentageValue();
        abstract public void setPercentageValue(int percentage);

	public void setIncrement(NumberType increment) {
		this.increment = increment;
	}

        protected String getStringValue() 
            { return printGUIValue(value); }

	abstract public void adjustValue(int amount);
}

