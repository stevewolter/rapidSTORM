/** 
 * MySwing: Advanced Swing Utilites 
 * Copyright (C) 2005  Santhosh Kumar T 
 * <p/> 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * <p/> 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *
 * Steve Wolter 2009:
 * I've modified this file slightly and moved it into the simparm
 * tree for ease of use.
 */ 

package au.com.pulo.kev.simparm;

import javax.swing.*;
import javax.swing.border.*;
import java.awt.event.*;
import java.awt.*;
 
public class ComponentTitledBorder implements Border, MouseListener, SwingConstants{ 
    int offset = 5; 
 
    Component comp; 
    JComponent container; 
    Rectangle rect; 
    Border border; 
 
    public ComponentTitledBorder(Component comp, JComponent container, Border border){ 
        this.comp = comp; 
        this.container = container; 
        this.border = border; 
        container.addMouseListener(this); 
    } 
 
    public boolean isBorderOpaque(){ 
        return true; 
    } 
 
    public void paintBorder(Component c, Graphics g, int x, int y, int width, int height){ 
        Insets borderInsets = border.getBorderInsets(c); 
        Insets insets = getBorderInsets(c); 
        int temp = (insets.top-borderInsets.top)/2; 
        border.paintBorder(c, g, x, y+temp, width, height-temp); 
        Dimension size = comp.getPreferredSize(); 
        rect = new Rectangle(offset, 0, size.width, size.height); 
        SwingUtilities.paintComponent(g, comp, (Container)c, rect); 
    } 
 
    public Insets getBorderInsets(Component c){ 
        Dimension size = comp.getPreferredSize(); 
        Insets insets = border.getBorderInsets(c); 
        insets.top = Math.max(insets.top, size.height); 
        return insets; 
    } 
 
    private void dispatchEvent(MouseEvent me){ 
        if(rect!=null && rect.contains(me.getX(), me.getY())){ 
            Point pt = me.getPoint(); 
            pt.translate(-offset, 0); 
            comp.setBounds(rect); 
            comp.dispatchEvent(new MouseEvent(comp, me.getID() 
                    , me.getWhen(), me.getModifiers() 
                    , pt.x, pt.y, me.getClickCount() 
                    , me.isPopupTrigger(), me.getButton())); 
            if(!comp.isValid()) 
                container.repaint(); 
        } 
    } 
 
    public void mouseClicked(MouseEvent me){ 
        dispatchEvent(me); 
    } 
 
    public void mouseEntered(MouseEvent me){ 
        dispatchEvent(me); 
    } 
 
    public void mouseExited(MouseEvent me){ 
        dispatchEvent(me); 
    } 
 
    public void mousePressed(MouseEvent me){ 
        dispatchEvent(me); 
    } 
 
    public void mouseReleased(MouseEvent me){ 
        dispatchEvent(me); 
    } 
}
