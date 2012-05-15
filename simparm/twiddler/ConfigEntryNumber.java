
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

package au.com.pulo.kev.simparm;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.text.NumberFormat;
import java.text.ParseException;
import java.io.*;
import java.util.StringTokenizer;
import javax.imageio.ImageIO;
import java.util.LinkedList;


abstract class ConfigEntryNumber
    extends ConfigEntry 
    implements NumberFieldListener
{
        private CardLayout card_panel_layout = new CardLayout();
        private JPanel master_panel = new JPanel(), card_panel = new JPanel(),
                       single_view_panel = new JPanel(), array_view_panel = new JPanel(),
                       array_panel = new JPanel();
        private JButton to_single = new JButton(), to_array = new JButton();

        private NumberField single_view;
        private NumberField[][] array_view;

        private int rows = 1, columns = 1;

        protected JLabel unitLabel = new JLabel();
        private boolean blockEvent;

        private NumberField had_a_change = null;

        private class NumberAcknowledgement extends Acknowledgement {
            NumberAcknowledgement() {
                blockEvent = true;
            }
            public void gotAcknowledgement() {
                blockEvent = false;
                if ( had_a_change != null ) {
                    NumberField c = had_a_change;
                    had_a_change = null;
                    contents_changed(c);
                }
            }
        };

        static ImageIcon chain = null, broken_chain = null;
        static final String single_view_name = "Single view", array_view_name = "Array view";

        private class SwitchToSingle implements ActionListener {
            public void actionPerformed(ActionEvent e) {
                card_panel_layout.show( card_panel, single_view_name );
            }
        }
        private class SwitchToArray implements ActionListener {
            public void actionPerformed(ActionEvent e) {
                card_panel_layout.show( card_panel, array_view_name );
            }
        }

        public ConfigEntryNumber() {
            super();
            if ( chain == null || broken_chain == null ) {
                try {
                    if ( chain == null )
                        chain = new ImageIcon( ImageIO.read(
                            getClass().getResourceAsStream("/stock-hchain-24.png"))); 
                    if ( broken_chain == null )
                         broken_chain =  new ImageIcon( ImageIO.read(getClass().getResourceAsStream("/stock-hchain-broken-24.png")));
                } catch ( IOException e ) {
                    System.err.println("Chain icons not found");
                }
            }
            single_view = new SliderNumberField( make_parser(), this );
            master_panel.setLayout(new BoxLayout(master_panel,BoxLayout.X_AXIS));

            card_panel.setLayout(card_panel_layout);

            master_panel.add(card_panel);
            master_panel.add(Box.createRigidArea(new Dimension(5,0)));
            master_panel.add(unitLabel);

            single_view_panel.setLayout( new BoxLayout(single_view_panel, BoxLayout.LINE_AXIS) );
            single_view_panel.add(single_view.get_component());
            to_array =  new JButton(chain);
            to_array.addActionListener( new SwitchToArray() );
            single_view_panel.add(to_array);
            card_panel.add( single_view_panel, "Single view" );

            array_view_panel.setLayout( new BoxLayout(array_view_panel, BoxLayout.LINE_AXIS) );
            to_single =  new JButton(broken_chain);
            to_single.addActionListener( new SwitchToSingle() );
            array_view_panel.add(array_panel);
            array_view_panel.add(to_single);
            card_panel.add( array_view_panel, "Array view" );
            setField(master_panel);

            make_array_view();
        }

        private void parse_matrix( String string, int field ) throws IOException {
            //if ( blockEvent && field == NumberField.Value ) return;
            StringTokenizer matrix = new StringTokenizer( string, "," );
            int row_count =  matrix.countTokens();
            String[][] rows = null;
            String constant_token = null;
            for (int i = 0; i < row_count; ++i) {
                StringTokenizer row = new StringTokenizer( matrix.nextToken(), " " );
                int columns = row.countTokens();
                if ( rows == null )
                    rows = new String[row_count][columns];
                else if ( columns != rows[0].length )
                    throw new IOException("Invalid matrix " + string + ": Column count is different for different rows");
                for (int j = 0; j < columns; ++j) {
                    rows[i][j] = row.nextToken();
                    if ( i == 0 && j == 0 )
                        constant_token = rows[i][j];
                    else if ( constant_token != null && ! constant_token.equals(rows[i][j]) )
                        constant_token = null;
                }
            }

            if ( constant_token != null )
                single_view.set_value_from_simparm( constant_token, field );
            if ( array_view.length != rows.length || array_view[0].length != rows[0].length )
                throw new IOException("The input matrix for " + getName() + " changed size. Sorry, this is not implemented.");

            blockEvent = true;
            for (int i = 0; i < rows.length; ++i)
                for (int j = 0; j < rows[i].length; ++j)
                    array_view[i][j].set_value_from_simparm( rows[i][j], field );
            blockEvent = false;
        }

    private void make_array_view() {
        array_view = new NumberField[rows][columns];
        to_array.setVisible( rows > 1 || columns > 1 );
        if ( rows == 1 && columns == 1 ) {
            /* Is no matrix after all, disable disjoint view */
            card_panel_layout.show( card_panel, single_view_name );
        } else if ( columns == 1 ) {
            /* Make vectors to row vectors to conserve space */
            array_panel.setLayout( new GridLayout( 1, rows ) );
        } else {
            array_panel.setLayout( new GridLayout( rows, columns ) );
        }
        array_panel.removeAll();
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < columns; ++j) {
                array_view[i][j] = new NumberField( make_parser(), this );
                array_panel.add( array_view[i][j].get_component() );
            }
    }

        private void set_availability( boolean is_available, int field ) {
            single_view.set_availability( is_available, field );
            if ( array_view != null )
                for (int i = 0; i < array_view.length; ++i)
                    for (int j = 0; j < array_view[i].length; ++j)
                        array_view[i][j].set_availability( is_available, field );
        }

	public boolean processAttribute(String name, String value)
            throws IOException
        {
            if (name.equals("value")) {
                if ( value != null ) parse_matrix( value, NumberField.Value );
                set_availability(  (value != null), NumberField.Value );
                setOptionalAvailableness( value != null );
            } else if (name.equals("increment")) {
                if ( value != null ) parse_matrix( value, NumberField.Increment );
                set_availability(  (value != null), NumberField.Increment );
            } else if (name.equals("min")) {
                if ( value != null ) parse_matrix( value, NumberField.Min );
                set_availability(  (value != null), NumberField.Min );
            } else if (name.equals("max")) {
                if ( value != null ) parse_matrix( value, NumberField.Max );
                set_availability(  (value != null), NumberField.Max );
            } else if (name.equals("unit_name")) {
                unitLabel.setToolTipText( value );
            } else if (name.equals("unit_symbol")) {
                unitLabel.setText( value );
            } else if (name.equals("rows")) {
                rows = Integer.parseInt(value);
                make_array_view();
            } else if (name.equals("columns")) {
                columns = Integer.parseInt(value);
                make_array_view();
            }
            else
                return super.processAttribute(name,value);

            return true;
        }


        public abstract NumberParser make_parser();
        
    public void contents_changed( NumberField in ) {
        if ( blockEvent )  {
            had_a_change = in;
            return;
        }
        if ( in  == single_view ) {
            String v = in.get_value(NumberField.Value);
            if ( v != null ) {
                for (int i = 0; i < array_view.length; ++i)
                    for (int j = 0; j < array_view[0].length; ++j) 
                        array_view[i][j].set_value_from_simparm( v, NumberField.Value );
            }
            enable_single_view(true);
        }
        String sv = getStringValue();
        if ( sv != null )
            super.print( "value set " + sv, new NumberAcknowledgement() );
    }

    private void enable_single_view( boolean yes ) {
        to_single.setEnabled( yes );
        if ( ! yes ) card_panel_layout.show( card_panel, array_view_name );
    }

    protected String getStringValue() { 
        String simparm_change = "";
        String constant_element = array_view[0][0].get_value(NumberField.Value);
        boolean all_the_same = true;
        for (int i = 0; i < array_view.length; ++i) {
            if ( i != 0 ) simparm_change = simparm_change + ",";
            for (int j = 0; j < array_view[0].length; ++j) {
                if ( j != 0 ) simparm_change = simparm_change + " ";
                String v = array_view[i][j].get_value(NumberField.Value);
                if ( v == null ) return null;
                simparm_change = simparm_change + v;
                if ( !v.equals( constant_element ) )
                    all_the_same = false;
            }
        }
        if ( all_the_same ) single_view.set_value_from_simparm( constant_element, NumberField.Value );
        enable_single_view( all_the_same );
        return simparm_change;
    }
        
    public void commitChanges() {
        /* This method needs no implementation since the events are handled by the text fields themselves */
    }

    public LinkedList<Component> getComponents() {
        LinkedList<Component> myComponents = new LinkedList<Component>();
        myComponents.add( to_single );
        myComponents.add( to_array );
        myComponents.add( unitLabel );
        myComponents.addAll( single_view.get_components() );
        if ( array_view != null )
            for (int i = 0; i < array_view.length; ++i)
                for (int j = 0; j < array_view[i].length; ++j)
                    myComponents.addAll( array_view[i][j].get_components() );
        return myComponents;
    }

}

