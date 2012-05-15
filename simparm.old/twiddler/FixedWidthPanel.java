package au.com.pulo.kev.simparm;

import java.awt.*;
import javax.swing.*;

public class FixedWidthPanel extends javax.swing.JPanel implements javax.swing.Scrollable {
    public boolean getScrollableTracksViewportWidth() {
        return true;
    }
    public boolean getScrollableTracksViewportHeight() {
        return false;
    }

     public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction) {
        return 1;
     }

     public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction) {
        return 1;
     }

     public Dimension getPreferredScrollableViewportSize() {
        return getPreferredSize();
     }
}
