import java.util.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.CardLayout;

class TreeComponent {
    private class PanelWrapper {
        public JComponent comp;
        public String toString() { return comp.getName(); }
    };

    private class Activator implements TreeSelectionListener {
        public void valueChanged(TreeSelectionEvent event) {
            DefaultMutableTreeNode node = (DefaultMutableTreeNode)
                event.getPath().getLastPathComponent();
            JComponent el = ((PanelWrapper)node.getUserObject()).comp;
            changeActiveComponent( el );
        }
    };

    private List<JComponent> addedBeforeShow
        = new LinkedList<JComponent>();
    private boolean wasShown = false;
    private JComponent activeComponent;

    public void addComponent(JComponent c) {
        if ( !wasShown ) {
            assert (c != null);
            addedBeforeShow.add( c );
        } else {
            String ident = Integer.toString( nextIdentity++ );
            identities.put( c, ident );
            rightField.add( c, ident );
        }
    }

    private DefaultTreeModel model;
    private JPanel rightField = new JPanel(new CardLayout());
    private JTree tree;
    private JSplitPane bothPanel;

    private int nextIdentity = 0;
    private HashMap<JComponent,String> identities
        = new HashMap<JComponent,String>();

    public JComponent wrapUserObject( DefaultMutableTreeNode node ) {
        PanelWrapper wrap = new PanelWrapper();
        wrap.comp = (JComponent)node.getUserObject();
        node.setUserObject( wrap );
        return wrap.comp;
    }
    public TreeComponent( DefaultMutableTreeNode rootNode ) {
        addComponent( wrapUserObject(rootNode) );
        model = new DefaultTreeModel( rootNode );
    }

    public void addNode( DefaultMutableTreeNode toAdd, 
                            DefaultMutableTreeNode addToParent,
                            boolean focus ) 
    {
        addComponent( wrapUserObject(toAdd) );

        model.insertNodeInto( toAdd, addToParent, 
            addToParent.getChildCount() );
        TreePath child = new TreePath(model.getPathToRoot( toAdd )),
                    parent = new TreePath( model.getPathToRoot( addToParent ));
        if ( tree != null )
            tree.expandPath( parent );
        /*if ( focus )
            tree.setSelectionPath( child );*/
        
    }
    public void removeNode( MutableTreeNode n ) {
        model.removeNodeFromParent(n);
    }

    public void notifyOfChangedDescription( MutableTreeNode changed ) 
        { model.nodeChanged( changed ); }

    public void changeActiveComponent( JComponent component ) { 
        activeComponent = component;
        if ( wasShown ) {
            int index = Integer.parseInt( identities.get(component) );
            CardLayout cl = (CardLayout)(rightField.getLayout());
            cl.first(rightField);
            while (index-- > 0)
                cl.next( rightField );
        }
    }

    public JComponent getComponent() { 
        if ( tree == null ) {
            tree = new JTree( model );
            tree.setEditable( true );
            tree.addTreeSelectionListener( new Activator() );
        }
        if ( bothPanel == null )
            bothPanel = new JSplitPane(
                JSplitPane.HORIZONTAL_SPLIT, tree, rightField);
        show();
        return bothPanel; 
    }

    public void show() { 
        if ( !wasShown ) {
            wasShown = true;
            if ( activeComponent != null )
                addComponent( activeComponent );
            for ( JComponent c : addedBeforeShow )
                if ( c != activeComponent )
                    addComponent( c );
            for (int i = 0; i < tree.getRowCount(); i++)
                tree.expandRow(i);
            addedBeforeShow = null;
        }
    }
};

