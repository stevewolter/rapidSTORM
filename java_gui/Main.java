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
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.util.LinkedList;

public class Main {
    public final static void main(String args[]) throws IOException {
        StartWithEnv(args, null, null);
    }

    public final static void StartWithEnv(String args[], String env[], String title)
        throws IOException 
    {
        final JFrame f = new JFrame( (title == null) ? "Twiddler" : title);
        JMenuBar menuBar = new JMenuBar();
        final Twiddler twiddler;

        if (args.length > 0) {
            twiddler = new Twiddler(args, env, menuBar);
        } else {
            String pid_str = Long.toString(Thread.currentThread().getId());
            String fifo_fname_out = ".control-" + pid_str + ".in";
            String fifo_fname_in = ".control-" + pid_str + ".out";
            Process mkfifo = Runtime.getRuntime().exec("mkfifo " + fifo_fname_out + " " + fifo_fname_in);
            int rc = -1;
            while (rc < 0) {
                try {
                            rc = mkfifo.waitFor();
                } catch (InterruptedException e) {
                }
            }
            if (rc != 0) {
                System.err.println("Error: unable to create control fifos");
                System.exit(rc);
            }

            twiddler = new Twiddler(fifo_fname_in, fifo_fname_out);
        }

        f.setJMenuBar(menuBar);

        f.addWindowListener(new WindowAdapter() {
            public final void windowClosing(WindowEvent e) {
                twiddler.close();
            }
        });

        f.add("Center", twiddler);
        if ( title == null ) {
            title = twiddler.getName();
            f.setTitle( title );
        }
        f.setDefaultCloseOperation( WindowConstants.DO_NOTHING_ON_CLOSE );
        f.pack();
        Dimension size = f.getPreferredSize();
        f.setSize(new Dimension( (int)(size.getWidth())+ 100, (int)(size.getHeight()) + 100 ) );
        f.setVisible(true);
        f.validate();

        twiddler.wasShown();

        if ( twiddler.monitorInput() )
            f.dispose();
        else
            f.setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
    }
}
