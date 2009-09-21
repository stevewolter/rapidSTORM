
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

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import java.awt.dnd.DropTargetListener;
import java.awt.dnd.DropTargetDragEvent;
import java.awt.dnd.DropTargetDropEvent;
import java.awt.dnd.DropTargetEvent;
import java.awt.dnd.DropTarget;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.DataFlavor;

class ConfigEntryFile extends ConfigEntryString 
implements ActionListener 
{
    private class Drop implements DropTargetListener {
        public void dragEnter(DropTargetDragEvent dtde) {}
        public void dragExit(DropTargetEvent dte) {}
        public void dragOver(DropTargetDragEvent dtde) {}
        public void dropActionChanged(DropTargetDragEvent dtde) {}

        public void drop(DropTargetDropEvent dtde) {
            Transferable t = dtde.getTransferable();
            if ( t.isDataFlavorSupported( DataFlavor.javaFileListFlavor ) ) {
                dtde.acceptDrop(  java.awt.dnd.DnDConstants.ACTION_COPY );
                try {
                    java.util.List l = (java.util.List) 
                        t.getTransferData( DataFlavor.javaFileListFlavor );
                    for ( Object f : l ) {
                        setValue( ((java.io.File)f).getAbsolutePath(),
                            true, true );
                    }
                } catch (Exception e) {
                    System.err.println(e);
                }
            }
        }
        
    };

        private JPanel panel;
        private JButton button;

	public ConfigEntryFile() {
		super();
                textField.setDragEnabled(true);
                new DropTarget( textField, new Drop() );
                panel = new JPanel();
                panel.setLayout(new BoxLayout(panel,BoxLayout.X_AXIS));
                panel.add(textField);
                button = new JButton("Select");
                button.addActionListener(this);
                button.setMaximumSize(button.getMinimumSize());
                panel.add(button);
                setField( panel );
	}

        public void actionPerformed(ActionEvent e) {
                if (e.getSource() == button) {
                	JFileChooser chooser = new JFileChooser(value);
                        int rv = chooser.showOpenDialog(panel);
                        if (rv == JFileChooser.APPROVE_OPTION) {
                           File file = chooser.getSelectedFile();
                           setValue(file.getPath(), true, true);
                        }

                } else
                  super.actionPerformed(e);
        }

        protected void updateEnabled() {
            super.updateEnabled();
            if (panel == null) return;
            if (textField != null)
                textField.setEnabled( panel.isEnabled() );
            if (button != null)
                button.setVisible( panel.isEnabled() );
        }

        public boolean processCommand(String line, BufferedReader rest)
            throws IOException 
        {
            if ( line.startsWith("extension") )
                return true;
            else 
                return super.processCommand(line,rest);
        }
}

