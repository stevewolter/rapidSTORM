
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
import java.util.HashMap;
import java.util.Map;
import java.util.LinkedList;
import java.util.Iterator;
import javax.swing.*;

public class Twiddler extends JPanel {
    private Process child;
    private Thread stderr_thread;
    private InputMonitor inputMonitor;
    private String configFile = null;

    private ConfigIO treeRoot;
    private int recordStart;

    public void setUserLevel(int newLevel) { 
        treeRoot.setRequiredUserLevel(newLevel);
    }

    private class DifficultyLevelSetter implements ItemListener {
        private Object o;
        private int dl;

        DifficultyLevelSetter(int level) { dl = level; }
        public void itemStateChanged(ItemEvent e) 
            { setUserLevel(dl); }
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

    private class ProcessWatcher extends Thread {
        public boolean is_stopped = false;
        public void run() { 
            int rv = -1;
            while ( ! is_stopped ) {
                try {
                    child.waitFor();
                    rv = child.exitValue();
                    child = null;
                    break;
                } catch (InterruptedException e) {}
            }
            if (rv != 0)
                JOptionPane.showMessageDialog
                    (null, "Program terminated unexpectedly with " +
                           "return code " + rv + ". This interface will " +
                           "stop reacting now, please the window and " +
                           "restart.", "Error",
                    JOptionPane.ERROR_MESSAGE);
        }
    };

    private void init(InputStream in, PrintStream out) throws IOException {
        treeRoot = new ConfigIO( 
            new BufferedReader(new InputStreamReader(in)), out );
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
            } catch (InterruptedException e) {}
            catch (java.lang.reflect.InvocationTargetException e) {
                e.getCause().printStackTrace();
                throw new RuntimeException(e.getCause().getMessage()); 
            }
        }
        if (attacher.except != null) throw attacher.except;
        
        this.setName( treeRoot.getDesc());
        add( treeRoot.configComponent() );
        validate();
    }

    public Twiddler(String args[]) throws IOException 
    {
        super(new GridLayout(1,1));
        int startArgs = 0;
        boolean stderrPipe = false;
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("--StderrPipe")) {
                stderrPipe = true;
            } else {
                startArgs = i;
                break;
            }
        }

        String[] reducedArgs = new String[args.length-startArgs];
        for (int i = startArgs; i < args.length; i++)
            reducedArgs[i-startArgs] = args[i];

        child = Runtime.getRuntime().exec(reducedArgs);
        new ProcessWatcher().start();

        PrintStream proxy = null;
        try {
            proxy = new PrintStream(child.getOutputStream());
            OutputStream os = child.getOutputStream();
            stderr_thread = (stderrPipe)
                ? new PipingThread( child.getErrorStream(), System.err )
                : new MessageDialogMaker( child.getErrorStream(), this );
            stderr_thread.start();

            init( child.getInputStream(), proxy );

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

            init(input, output);
    }

    public void monitorInput() {
        inputMonitor = new InputMonitor();
        inputMonitor.start();
        boolean joined = false;
        while ( ! joined )
            try {
                inputMonitor.join();
                joined = true;
            } catch(InterruptedException exc) {}

        if (treeRoot != null) treeRoot.detach();

        if (child != null) {
            int rc = -1;
            do {
                try { rc = child.waitFor(); }
                catch (InterruptedException exc) {}
            } while (rc < 0);
            try {
                child.getErrorStream().close();
            } catch (IOException e) {
            } catch (NullPointerException e) {}
        }
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
            HelpManager.openManual();
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

        load.addActionListener( new Loader(save) );
        save.addActionListener( new Saver(false, save) );
        saveAs.addActionListener( new Saver(true, save) );
        save.setEnabled(false);

        file.add(load);
        file.add(save);
        file.add(saveAs);

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
        result.add(help);

        return result;
    }

    public void close() {
        treeRoot.close();
    }

    public void wasShown() {
        treeRoot.wasShown();
    }
}
