import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import javax.swing.BoxLayout;
import javax.swing.JPanel;
import javax.swing.JComponent;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.awt.Insets;

abstract class DescriptedPanel extends ConfigObject 
implements MouseListener 
{
    private int sideLabels = 0;
    private boolean isSideLabelLayout = false;
    protected JPanel panel = new JPanel();

    DescriptedPanel() {
        panel.setLayout( new BoxLayout( panel, BoxLayout.PAGE_AXIS ) );
    }

    public void setDesc(String desc) {
        super.setDesc(desc);
        panel.setName(desc);
    }

    public void mouseClicked(MouseEvent e) { }
    public void mousePressed(MouseEvent e) { }
    public void mouseReleased(MouseEvent e) { }

    public void mouseEntered(MouseEvent e) {
        /*
        if (label instanceof JLabel) {
                //label.setFont(label.getFont().deriveFont(Font.BOLD));
        }*/
    }

    public void mouseExited(MouseEvent e) {
        /*
        if (label instanceof JLabel) {
                //label.setFont(label.getFont().deriveFont(Font.PLAIN));
        }
        */
    }

    protected JComponent getField() { 
        return panel; 
    }   
    
    public void add(ConfigObject entry) {
        super.add(entry);
        entry.setSelfDescribing( ! needsSideLabelLayout() );
        if ( entry.hasLabel() )
            sideLabels++;
        checkLayout();
        addToField(entry.getLabel(), entry.getField(),
                        entry.fieldShouldExpand() );
    }

    public void remove(ConfigObject entry) {
        super.remove(entry);
        if ( entry instanceof ConfigEntry )
            sideLabels++;
        checkLayout();
        removeFromField(entry.getLabel(), entry.getField());
    }

    private boolean needsSideLabelLayout() { return sideLabels > 0; }
    private void checkLayout() {
        if ( isSideLabelLayout != needsSideLabelLayout() ) {
            for ( ConfigObject o : children )
                o.setSelfDescribing( ! needsSideLabelLayout() );
        }
        if ( isSideLabelLayout && ! needsSideLabelLayout() ) {
            panel.setLayout( new BoxLayout( panel, BoxLayout.PAGE_AXIS ) );
        } else if ( ! isSideLabelLayout && needsSideLabelLayout() ) {
            GridBagLayout gridbag = new GridBagLayout();
            int[] widths = new int[2];
            widths[0] = 300; widths[1] = 0;
            gridbag.columnWidths = widths;
            panel.setLayout( gridbag );
        }
        isSideLabelLayout = needsSideLabelLayout();
    }

    protected void addToField(JComponent label, JComponent field,
         boolean shouldExpand)
    {
        GridBagConstraints c = new GridBagConstraints();
        c.fill = GridBagConstraints.HORIZONTAL;
        c.insets = new Insets(2,2,2,20);
        if (label != null) { panel.add( label, c ); }
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.weightx = 1;
        c.insets = new Insets(2,2,2,2);
        if (shouldExpand) c.weighty = 1;

        if ( field != null ) panel.add( field,c );
        panel.validate();
    }

    protected void removeFromField(JComponent label, JComponent field)
    {
        if (label != null) panel.remove(label);
        if (field != null) panel.remove(field);
        panel.validate();
    }
}
