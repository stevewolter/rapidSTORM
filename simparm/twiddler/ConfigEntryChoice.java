
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
        static class Choice {
            public String name;
            public String desc;
            public String toString() { return desc; }
        };
        private Map<String,Choice> choices = new HashMap<String,Choice>();

	public String value = null;

	public ConfigEntryChoice() {
		super();
		value = "";
	}

        protected String getStringValue() { return value; }

	public void setValue(Object newvalue, boolean doprint,
                             boolean updatefield) 
        {
            String nvname = "";
            if ( newvalue instanceof Choice )
                nvname = ((Choice)newvalue).name;
            if (nvname != value) {
                value = nvname;
                if (doprint) {
                    doPrint();
                }
            }
            if (updatefield) {
                select(newvalue);
            }
	}

        public void setValueFromWord(String word) 
            throws IOException
        {
            if (word.equals(""))
                setValue(null, false, true);
            else {
                Object choice = choices.get(word);
                if (choice != null) {
                    setValue(choice, false, true);
                } else {
                    throw new IOException("Unknown choice " + word);
                }
            }
        }

        private void addChoice(String name, Choice o) {
            choices.put(name, o);
            addChoice(o);
        }

        protected void removeChoice(String name) {
            super.removeChoice( choices.get(name) );
            choices.remove(name);
        }

        protected void clearChoices() {
            super.clearChoices();
            choices.clear();
        }

        public boolean processCommand(String line, BufferedReader stream)
            throws IOException 
        {
            String[] split = line.split(" ", 2);

            if (line.equals("declare choices")) {
                while (true) {
                    try {
                        String nline = stream.readLine();
                        if ( nline.equals("end") )
                            break;
                        else if ( nline.equals("declare choice") )
                            parseChoice(stream);
                        else
                            System.err.println("Unexpected line in " +
                             "choices declaration: " + nline);
                    } catch (IOException e) {
                        System.err.println(e);
                    }
                }
            } else if (split.length < 2) {
                setValueFromWord(split[1]);
            } else if ( isValue(split[0])) {
                setValueFromWord(split[1]);
            } else if (split[0].equals("choice")) {
                /* Changes in the AllChoices subset */
                split = split[1].split(" ", 2);
                if (split[0].equals("declare")) {
                    parseChoice(stream);
                } else if (split[0].equals("remove")) {
                    removeChoice(split[1]);
                } else
                    System.err.println("Unimplemented choice command " +
                        split[0]);
            } else
                return super.processCommand(line, stream);

            return true;
        }
        public void parseChoice(BufferedReader stream) {
            Choice newChoice = new Choice();
            while ( true ) {
                String nextLine = "end";
                try {
                    nextLine = stream.readLine();
                } catch (IOException e) {
                    System.err.println(e);
                }
                if (nextLine.equals("end"))
                    break;
                else {
                    String[] split = nextLine.split(" ", 2);
                    if (split.length == 2) {
                        if (split[0].equals("name"))
                            newChoice.name = split[1];
                        if (split[0].equals("desc"))
                            newChoice.desc = split[1];
                    }
                }
            }
            addChoice(newChoice.name, newChoice);
        }

}

