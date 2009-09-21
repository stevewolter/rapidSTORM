
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

import java.io.*;
import java.util.*;


class ConfigEntryChoice extends ConfigEntryChooser {
        class Choice implements ConfigObject.Listener {
            public String name, desc;
            public int userLevel;
            private boolean madeUserLevel = false;
            public ConfigObject node;

            public String toString() { return desc; }

            public void notifyOfAddition(ConfigObject parent, 
                                        ConfigObject child) 
                {}
            public void notifyOfRemoval(ConfigObject parent,
                                        ConfigObject child)
                {}
            public void notifyOfVisibilityChange(ConfigObject o) {}
            public void notifyOfNewDescription
                (ConfigObject o, String desc) 
            {
                this.desc = desc;
            }
            
            public void setRequiredUserLevel(int requiredLevel) {
                boolean canMakeUserLevel = ( requiredLevel >= userLevel );
                if ( canMakeUserLevel && !madeUserLevel )
                    addChoice( name, this );
                else if ( !canMakeUserLevel && madeUserLevel )
                    removeChoice( name );
                madeUserLevel = canMakeUserLevel;
            }

            public List<SubPanel> subpanels = new LinkedList<SubPanel>();
            public Map<SubPanel,Integer> offsets
                = new HashMap<SubPanel,Integer>();
        };

        void commitChanges() {}

        private Map<String,Choice> choices = new HashMap<String,Choice>();

	public String value = null, displayedName = null;

	public ConfigEntryChoice() {
		super();
	}

        protected String getStringValue() { return value; }

	public void setValue(Object newvalue, boolean doprint,
                             boolean updatefield) 
        {
            String nvname = null;
            if ( newvalue instanceof Choice )
                nvname = ((Choice)newvalue).name;

            if (nvname != value) {
                value = nvname;
                if (doprint) {
                    doPrint();
                }
            }
            if (updatefield) {
                changeFields(nvname);
                select(newvalue);
            }
	}

        public void setValueFromWord(String word) 
            throws IOException
        {
            if (word.equals("")) {
                setValue(null, false, true);
            } else {
                Object choice = choices.get(word);
                if (choice != null) {
                    setValue(choice, false, true);
                } else
                    value = word;
            }
        }

        private void addChoiceFromObject(ConfigObject o) {
            Choice c = choices.get( o.getName() );
            if ( c == null ) {
                c = new Choice();
                c.name = o.getName();
                choices.put( c.name, c );
            }
            c.desc = o.getDesc();
            c.userLevel = o.getUserLevel();
            c.node = o;
            o.addListener( c );

            c.setRequiredUserLevel( getRequiredUserLevel() );
        }

        private void addChoice(String name, Choice o) {
            choices.put(name, o);
            addChoice(o);
            if ( name.equals(value) ) {
                setValue(o, false, true);
            }
        }

        protected void removeChoice(String name) {
            super.removeChoice( choices.get(name) );
            choices.remove(name);
        }

        protected void clearChoices() {
            super.clearChoices();
            choices.clear();
        }

        public void add(ConfigObject entry) {
            super.add(entry);
            addChoiceFromObject( entry );
        }

        public void remove(ConfigObject entry) {
            String name = entry.getName();
            removeChoice(name);
            super.remove(entry);
        }

        public boolean processCommand(String line, BufferedReader stream)
            throws IOException 
        {
            String[] split = line.split(" ", 2);

            if (split.length < 2) {
                setValueFromWord(split[1]);
            } else if ( isValue(split[0])) {
                setValueFromWord(split[1]);
            } else
                return super.processCommand(line, stream);

            return true;
        }

    public void setRequiredUserLevel(int requiredLevel) {
        super.setRequiredUserLevel(requiredLevel);

        boolean completed_run = false;
        while ( !completed_run) {
            try {
                for ( Choice c : choices.values() )
                    c.setRequiredUserLevel( requiredLevel );
                completed_run = true;
            } catch (java.util.ConcurrentModificationException e) {}
        }
    }

    public void subPanelChange( ConfigObject.SubPanel from, 
                                ConfigObject.SubPanel to,
                                ConfigObject fromNode, int offset_in_node) 
    {
        String name = fromNode.getName();
        Choice choice = choices.get( name );
        if ( choice == null ) {
            choice = new Choice();
            choice.name = name;
            choices.put( name, choice );
        }
        if ( from != null ) choice.subpanels.remove( from );
        if ( to != null ) {
            choice.subpanels.add( to );
            choice.offsets.put( to, new Integer( offset_in_node ) );
        }
        if ( name == value )
            super.subPanelChange( from, to, fromNode, offset_in_node );
    }

    public void changeFields(String newV) {
        if ( displayedName != null ) {
            Choice o = choices.get( displayedName );
            for ( SubPanel s : o.subpanels ) {
                super.subPanelChange( 
                    s, null, o.node, o.offsets.get(s).intValue());
            }
        }
        displayedName = newV;
        if ( newV != null ) {
            Choice o = choices.get( newV );
            if ( o != null )
                for ( SubPanel s : o.subpanels ) {
                    super.subPanelChange( 
                        null, s, o.node, o.offsets.get(s).intValue());
                }
        }
    }
}

