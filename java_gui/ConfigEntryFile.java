
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
import java.awt.dnd.DropTargetListener;
import java.awt.dnd.DropTargetDragEvent;
import java.awt.dnd.DropTargetDropEvent;
import java.awt.dnd.DropTargetEvent;
import java.awt.dnd.DropTarget;
import java.awt.datatransfer.Transferable;
import java.awt.datatransfer.DataFlavor;

public class ConfigEntryFile extends ConfigEntryString 
implements ActionListener 
{
    public static java.io.File basePath;

    static File lastSelectedFile = null;

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
                        chooser.setCurrentDirectory( lastSelectedFile );
                        int rv = chooser.showOpenDialog(panel);
                        if (rv == JFileChooser.APPROVE_OPTION) {
                           File file = chooser.getSelectedFile();
                           lastSelectedFile = file;
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

        public boolean processAttribute(String name, String value)
            throws IOException 
        {
            if ( name.equals("extension") )
                return true;
            else 
                return super.processAttribute(name,value);
        }

        protected void declareDescriptedPanels(DeclarationInfo info) {
            if ( info != null && info.menu != null ) {
                JMenuItem item = new JMenuItem(getDesc());
                item.addActionListener( new ActionListener() { 
                    public void actionPerformed(ActionEvent e) {
                      try {
                        java.awt.Desktop.getDesktop().open( 
                            new java.io.File(basePath,
                                value.replace('/', java.io.File.pathSeparatorChar ) ) );
                      } catch (Exception ex) {}
                    }
                } );
                info.menu.add(item);
            } else {
                super.declareDescriptedPanels( info );
            }
        }
}

