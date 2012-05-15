
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
import java.util.HashMap;
import java.util.Map;
import java.util.LinkedList;
import java.util.Iterator;
import javax.swing.*;

public class Twiddler extends JPanel {
    private Process child;
    boolean tried_to_close = false, expect_child_error = false;
    private Thread stderr_thread;
    private InputMonitor inputMonitor;
    private String configFile = null;

    private ConfigIO treeRoot;
    private int recordStart;

    public void setUserLevel(int newLevel) { 
        if ( treeRoot != null )
            treeRoot.setRequiredUserLevel(newLevel);
    }

    public static interface DefaultsSaver {
        public abstract void save_defaults( java.util.List<String> commands );
    }
    private static DefaultsSaver defaults_saver = null;
    public static void setDefaultsSaver( DefaultsSaver s )
        { defaults_saver = s; }

    private class DifficultyLevelSetter implements ItemListener {
        private Object o;
        private int dl;

        DifficultyLevelSetter(int level) { dl = level; }
        public void itemStateChanged(ItemEvent e) 
            { setUserLevel(dl); }
    }

    private class DefaultsSaverAction implements ActionListener {
        public void actionPerformed(ActionEvent e) {
            defaults_saver.save_defaults( treeRoot.getCommandHistory(recordStart) );
        }
    }

    private class Saver implements ActionListener {
        private boolean as;
        private JMenuItem save;
        public Saver(boolean as, JMenuItem s) { this.as = as; save = s; }
        public void actionPerformed(ActionEvent e) { 
            String fn = null;
            if (as) {
                final JFileChooser fc = new JFileChooser();
                int returnVal = fc.showSaveDialog(Twiddler.this);

                if (returnVal == JFileChooser.APPROVE_OPTION) 
                    fn = fc.getSelectedFile().getAbsolutePath(); 
            } else
                fn = configFile;

            if (fn != null) {
                java.util.List<String> toSave =
                  treeRoot.getCommandHistory(recordStart);
                try {
                    PrintWriter writer = new PrintWriter(fn);
                    for ( String line : toSave )
                        writer.println( line );
                    writer.flush();
                    writer.close();
                } catch ( FileNotFoundException err ) {
                    JOptionPane.showMessageDialog
                    (null, err, "Error",
                    JOptionPane.ERROR_MESSAGE);
                }
                if (configFile == null) 
                    { configFile = fn; save.setEnabled(true); }
            }
        }
    }

    private class Loader implements ActionListener {
        private JMenuItem save;
        public Loader(JMenuItem s) { save = s; }
        public void actionPerformed(ActionEvent e) { 
            final JFileChooser fc = new JFileChooser();
            int returnVal = fc.showOpenDialog(Twiddler.this);

            if (returnVal == JFileChooser.APPROVE_OPTION) {
                configFile = fc.getSelectedFile().getAbsolutePath(); 

                treeRoot.loadConfigFromFile(configFile);

                save.setEnabled(true);
            }
        }
    }

    private class ExceptionRunnable implements Runnable {
        public IOException except = null;
        public void run() { 
            try {
                treeRoot.attach(); 
            } catch (IOException e) { except = e; }
        }
    }

    private void init(InputStream in, PrintStream out, boolean echo_to_stdout, 
                   JMenuBar menubar )
        throws IOException 
    {
        if ( menubar != null ) {
            LinkedList<JMenu> menus = makeMenus();
            for ( JMenu menu : menus )
                menubar.add(menu);
        }

        treeRoot = new ConfigIO( 
            new BufferedReader(new InputStreamReader(in)), out, echo_to_stdout, menubar );
         recordStart = treeRoot.getCommandHistoryPosition();

        int m = 8;
        setBorder(BorderFactory.createEmptyBorder(m, m, m, m));

        IOException except = null;
        ExceptionRunnable attacher = new ExceptionRunnable();
        
        boolean ran = false;
        while ( !ran ) {
            try {
                SwingUtilities.invokeAndWait(attacher);
                ran = true;
            } catch (InterruptedException e) {
            } catch (java.lang.reflect.InvocationTargetException e) {
                e.getCause().printStackTrace();
                throw new RuntimeException(e.getCause().getMessage()); 
            }
        }
        if (attacher.except != null) throw attacher.except;
        
        this.setName( treeRoot.getDesc());
        add( treeRoot.configComponent() );
        validate();
    }

    public Twiddler(String args[], String env[], JMenuBar menuBar) throws IOException 
    {
        super(new GridLayout(1,1));
        int startArgs = 0;
        boolean stderrPipe = false, stdoutPipe = false;
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("--StderrPipe")) {
                stderrPipe = true;
            } else if (args[i].equals("--StdoutPipe")) {
                stdoutPipe = true;
            } else {
                startArgs = i;
                break;
            }
        }

        String[] reducedArgs = new String[args.length-startArgs];
        for (int i = startArgs; i < args.length; i++)
            reducedArgs[i-startArgs] = args[i];

        try {
            child = Runtime.getRuntime().exec(reducedArgs, env);
        } catch (IOException e) {
            reducedArgs[0].concat(".exe");
            child = Runtime.getRuntime().exec(reducedArgs, env);
        }

        PrintStream proxy = null;
        try {
            proxy = new PrintStream(child.getOutputStream());
            OutputStream os = child.getOutputStream();
            stderr_thread = (stderrPipe)
                ? new PipingThread( child.getErrorStream(), System.err )
                : new MessageDialogMaker( child.getErrorStream(), this );
            stderr_thread.start();

            init( child.getInputStream(), proxy, stdoutPipe, menuBar );

        } catch (IOException e) {
            int rc = -1;
            if (proxy != null) {
                proxy.println("detach");
                proxy.println("quit");
                proxy.close();
            }
            do {
                try { rc = child.waitFor(); }
                catch (InterruptedException exc) {}
            } while (rc < 0);
            throw e;
        }
    }

    public Twiddler(String inputFile, String outputFile) throws IOException
    {
        super(new GridLayout(1,1));
            System.err.print("Waiting to connect to " + outputFile + 
                             "... ");
            System.err.flush();
            PrintStream output = 
                new PrintStream(new FileOutputStream(outputFile));
            System.err.println("connected");
            System.err.print("Waiting to connect to " + 
                             inputFile + "... ");
            System.err.flush();
            InputStream input = new FileInputStream(inputFile);
            System.err.println("done");

            init(input, output, false, null);
    }

    public boolean monitorInput() {
        inputMonitor = new InputMonitor();
        inputMonitor.start();
        while ( inputMonitor != null )
            try {
                inputMonitor.join();
                inputMonitor = null;
            } catch(InterruptedException exc) {}

        if (treeRoot != null) treeRoot.detach();
        
        boolean exit_with_error = false;
        while ( child != null ) {
            try {
                int rv = child.waitFor();
                exit_with_error = (rv != 0);
                child = null;
            } catch ( InterruptedException e ) {}
        }

        return !exit_with_error;
    }

    private class InputMonitor extends Thread {
        public void run() {
            try {
                treeRoot.monitorInput();
            } catch (IOException e) {
                JOptionPane.showMessageDialog
                    (null, e.getMessage(), "Error",
                    JOptionPane.ERROR_MESSAGE);
            }
        }

    }

    private class ManualOpener implements ActionListener {
        public void actionPerformed(ActionEvent e) { 
            HelpManager.getSingleton().openManual();
        }
    }

    private class WhatsThis implements ActionListener {
        public void actionPerformed(ActionEvent e) { 
            JOptionPane.showMessageDialog(null,
              "Right-click on a field or input element to get help.");
        }
    }

    public LinkedList<JMenu> makeMenus() {
        LinkedList<JMenu> result = new LinkedList<JMenu>();

        JMenu file = new JMenu("Macro");
        JMenuItem load = new JMenuItem("Load ...");
        JMenuItem save = new JMenuItem("Save");
        JMenuItem saveAs = new JMenuItem("Save as ...");
        JMenuItem saveAsDefault = new JMenuItem("Save as default");

        load.addActionListener( new Loader(save) );
        save.addActionListener( new Saver(false, save) );
        saveAs.addActionListener( new Saver(true, save) );
        saveAsDefault.addActionListener( new DefaultsSaverAction() );
        save.setEnabled(false);

        file.add(load);
        file.add(save);
        file.add(saveAs);
        if ( defaults_saver != null ) file.add(saveAsDefault);

        result.add(file);

        JMenu difficulty = new JMenu("Expertise");
        ButtonGroup group = new ButtonGroup();
        final JMenuItem easy = new JRadioButtonMenuItem("Casual user");
        JMenuItem normal = new JRadioButtonMenuItem("Normal operator");
        JMenuItem expert = new JRadioButtonMenuItem("Expert");

        easy.addItemListener( new DifficultyLevelSetter(10) );
        normal.addItemListener( new DifficultyLevelSetter(20) );
        expert.addItemListener( new DifficultyLevelSetter(30) );

        group.add(easy); difficulty.add(easy);
        group.add(normal); difficulty.add(normal);
        group.add(expert); difficulty.add(expert);
        EventQueue.invokeLater( new Runnable() {
            public void run() { easy.doClick(); } } );

        result.add(difficulty);

        JMenu help = new JMenu("Help");
        JMenuItem manual = new JMenuItem("Manual");
        JMenuItem context_sens = new JMenuItem("What's this?");
        manual.addActionListener(new ManualOpener());
        context_sens.addActionListener(new WhatsThis());
        help.add( manual );
        help.add( context_sens );
        JMenuItem[] more_items = HelpManager.getSingleton().getExtraURIs();
        for (int i = 0; i < more_items.length; ++i)
            help.add( more_items[i] );
        result.add(help);

        return result;
    }

    public void close() {
        if ( tried_to_close && child != null ) {
            int option = JOptionPane.showConfirmDialog(null,
                "You have already signalled the program that it should\n"
               +"close itself. It did not fulfill that request yet and\n"
               +"there might be reasons for that, such as necessary cleanup \n"
               +"or hardware shutdown. You have the option to forcibly\n"
               +"terminate the program, but that would cancel these\n"
               +"operations.\n\nTerminate program?",
                "Confirm process termination",
                JOptionPane.YES_NO_OPTION );
            if ( option == JOptionPane.YES_OPTION ) {
                expect_child_error = true;
                child.destroy();
            }
        } else if ( child != null ) {
            tried_to_close = true;
            treeRoot.close();
        }
    }

    public void wasShown() {
        treeRoot.wasShown();
    }
}
