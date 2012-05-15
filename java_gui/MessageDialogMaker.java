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

import java.io.*;
import javax.swing.*;
import java.awt.*;

public class MessageDialogMaker extends Thread {
    private BufferedReader from;
    private JComponent parent;

    public MessageDialogMaker(InputStream from, JComponent c) {
        this.from
        = new BufferedReader(new InputStreamReader(from));
        parent = c;
    }

    public void run() {
        try {
            String s;
            while ( (s = from.readLine()) != null) {
                java.util.LinkedList<String> list
                    = new java.util.LinkedList<String>();
                String line = new String("");
                while ( ! s.equals("") ) {
                    String[] split = s.split(" ", 2);
                    if ( (line.length() + split[0].length()) < 60 ) {
                        line = line.concat(" ").concat(split[0]);
                    } else {
                        list.add(line);
                        line = split[0];
                    }
                    s = (split.length == 2) 
                            ? split[1] : new String("");
                }
                if ( !line.equals("") )
                    list.add( line );
                
                JOptionPane.showMessageDialog(parent, list.toArray(),
                        "Error", JOptionPane.ERROR_MESSAGE);
            }
        } catch(IOException e) {
            System.err.println(e);
        }
    }
}

