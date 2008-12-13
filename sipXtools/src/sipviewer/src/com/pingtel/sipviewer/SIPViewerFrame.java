package com.pingtel.sipviewer ;

import javax.swing.* ;
import javax.swing.table.* ;
import javax.swing.border.* ;
import javax.swing.event.* ;
import java.awt.* ;
import java.awt.event.* ;
import java.lang.* ;
import java.util.* ;
import java.io.* ;
import java.net.* ;
import java.awt.datatransfer.*;


public class SIPViewerFrame extends JFrame
{
////////////////////////////////////////////////////////////////////////
// Constants
////


////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected SIPChartModel  m_model ;
    protected ChartBody      m_body ;
    protected ChartHeader    m_header ;
    protected JScrollPane    m_scrollPane ;
    protected SIPInfoPanel   m_infoPanel ;
    protected String m_fileChooserDir;
    protected boolean m_sortBranchNodes;
    // Initialize with a trivial reload object so Reload does nothing.
    protected Reload         m_Reload = new Reload();

    public SIPViewerFrame()
    {
        super("SIP Viewer") ;
        setSize(800, 600) ;
        m_sortBranchNodes = false;
        addWindowListener(new icWindowAdapter()) ;

        createComponents() ;
        layoutComponents() ;
        initMenu() ;
    }


    /*
     *
     */
    public void applyAliasesFile(String strAliasesFile)
    {
        try
        {
            FileReader fr = new FileReader(strAliasesFile) ;

            BufferedReader reader = new BufferedReader(fr) ;
            String strLine = reader.readLine() ;
            while (strLine != null)
            {
                int pos = strLine.indexOf("=") ;
                if (pos > 0)
                {
                    String strValue = strLine.substring(0, pos) ;
                    String strKey = strLine.substring(pos+1) ;

                    strKey = strKey.trim() ;
                    strValue = strValue.trim() ;

                    // System.out.println("AddAlias: " + strValue + " -> " + strKey) ;

                    m_model.removeKey(strValue);
                    m_model.addKeyAlias(strKey, strValue) ;
                    m_model.reindexData();
                }

                strLine = reader.readLine() ;
            }

            reader.close();
            fr.close() ;
        }
        catch (Exception e)
        {
            System.out.println("Unable to apply aliases file: " + strAliasesFile) ;
            e.printStackTrace() ;
            System.exit(-1);
        }
    }

    public void applySourceFile(String strSourceFile)
    {
        Vector vData = SipBranchData.getSipBranchDataElements(strSourceFile) ;
        applyData(vData) ;
    }

    protected void applyData(Vector vData)
    {
        m_model.clear() ;

        if(m_sortBranchNodes)
        {
            // Do some magic to adjust for the case where the response
            // shows up just before the request.
            SipBranchData current = null ;
            SipBranchData previous = null ;
            String strCurrentBranchId ;
            String strPreviousBranchId ;
            for (int i=vData.size()-1; i>0; i--)
            {
                current = (SipBranchData) vData.elementAt(i) ;
                strCurrentBranchId = current.getThisBranchId() ;

                previous = (SipBranchData) vData.elementAt(i-1) ;
                strPreviousBranchId = current.getThisBranchId() ;

                if ( (strCurrentBranchId != null)
                    && (strCurrentBranchId.length() > 0)
                    && (strPreviousBranchId != null)
                    && (strPreviousBranchId.length() > 0)
                    && (strCurrentBranchId.equals(strPreviousBranchId))
                    && (current.isRequest())
                    && (!previous.isRequest())
                    && (current.getMethod() != "ack"))
                {
                    vData.set(i, previous) ;
                    vData.set(i-1, current) ;

                    System.out.println("Req/Resp swapped for branchID: " + strCurrentBranchId) ;
                }
            }
        }

        SipBranchData data = null ;
        for (int i=0; i<vData.size(); i++)
        {
            data = (SipBranchData) vData.elementAt(i) ;
            addEntryToModel(data) ;
        }

        // Reset the scroll bar to the top.
        JScrollBar scroll = m_scrollPane.getVerticalScrollBar();
        scroll.setValue(scroll.getMinimum());

        // Revalidate to make sure the drawing pane is resized to match the
        // data.
        m_body.revalidate() ;
    }


    protected void addEntryToModel(SipBranchData data)
    {
        String strSourceAliases = null ;
        String strTargetAliases = null ;
        String strLabel = "Error" ;
        String strSource ;
        String strTarget ;

        if (data.getSourceEntity() != null)
        {
            strSource = data.getSourceEntity() ;
            strSourceAliases = data.getSourceAddress() ;
        }
        else if (data.getSourceAddress() != null)
        {
            strSource = data.getSourceAddress() ;
        }
        else
            strSource = "Unknown" ;

        if (data.getDestinationEntity() != null)
        {
            strTarget = data.getDestinationEntity() ;
            strTargetAliases = data.getDestinationEntity() ;
        }
        else if (data.getDestinationAddress() != null)
        {
            strTarget = data.getDestinationAddress() ;
        }
        else
            strTarget = "Unknown" ;

        if (data.isRequest())
        {
            strLabel = data.getMethod() ;
        }
        else
        {
            strLabel = data.getResponseCode() + " " + data.getResponseText() ;
        }

        m_model.addEntry(strSource, strTarget, strLabel, data) ;
        if (strSourceAliases != null)
            m_model.addKeyAlias(strSource, strSourceAliases) ;
        if (strTargetAliases != null)
            m_model.addKeyAlias(strTarget, strTargetAliases) ;
    }


    protected void createComponents()
    {
        m_model = new SIPChartModel() ;
        m_infoPanel = new SIPInfoPanel() ;

        m_header = new ChartHeader(m_model, m_infoPanel) ;
        m_body = new ChartBody(m_model, m_infoPanel) ;

        m_scrollPane = new JScrollPane(m_body, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED) ;
        m_scrollPane.setColumnHeaderView(m_header) ;
        // Make the viewport's background color black, in case the displayed
        // data panel is smaller than the viewport.
        m_scrollPane.getViewport().setBackground(Color.black) ;
    }


    protected void layoutComponents()
    {
        Container rootPane = getContentPane() ;

        Container tempCont = new Container() ;
        tempCont.setLayout(new GridBagLayout()) ;

        GridBagConstraints gbc = new GridBagConstraints() ;
        gbc.weightx = 1.0 ;
        gbc.weighty = 1.0 ;
        gbc.fill = GridBagConstraints.BOTH ;
        gbc.gridwidth = GridBagConstraints.REMAINDER ;
        tempCont.add(m_scrollPane, gbc) ;

        gbc.weightx = 1.0 ;
        gbc.weighty = 0.0 ;
        gbc.gridwidth = 1 ;
        gbc.fill = GridBagConstraints.HORIZONTAL ;
        tempCont.add(m_infoPanel, gbc) ;

        rootPane.add(tempCont, BorderLayout.CENTER) ;
    }


    protected JFrame getFrame()
    {
        return this ;
    }


    protected class icWindowAdapter extends WindowAdapter
    {
        public void windowOpened(WindowEvent e)
        {

        }

        public void windowClosing(WindowEvent e)
        {
            System.exit(1) ;
        }
    }


    protected String selectFile(JFrame parent, String strFilter, String strFilterDesc)
    {
        String fileName = null;
        if(m_fileChooserDir == null ||
           m_fileChooserDir.length() < 1)
        {
            m_fileChooserDir = ".";
        }
        System.out.println("chooser dir: " + m_fileChooserDir);
        JFileChooser chooser = new JFileChooser(m_fileChooserDir);


        ExampleFileFilter filter = new ExampleFileFilter() ;
        filter.addExtension(strFilter);
        filter.setDescription(strFilterDesc);
        chooser.setFileFilter(filter);

        int returnVal = chooser.showOpenDialog(parent);
        if(returnVal == JFileChooser.APPROVE_OPTION)
        {
            File file = chooser.getSelectedFile();
            fileName = file.getAbsolutePath();

            // Set the default directory for next time
            m_fileChooserDir = file.getParentFile().getAbsolutePath();
            System.out.println("saving chooser dir: " + m_fileChooserDir);

            String fileNameNoPath = file.getName();
            setTitle("SIP Viewer - " + fileNameNoPath);
        }
        return(fileName);
    }


    protected void initMenu()
    {
        JMenu menu;
        JMenuItem menuItem;

        // Create the menu bar.
        JMenuBar menuBar = new JMenuBar();
        setJMenuBar(menuBar);

        // Build the File menu.
        menu = new JMenu("File");
        //menu.setMnemonic(KeyEvent.VK_A);
        menuBar.add(menu);

        // Add the load-file items to the File menu.
        menuItem = new JMenuItem();
        menuItem.setAction(new icOpenFileAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F,
                                                       ActionEvent.ALT_MASK));
        menu.add(menuItem);

        menuItem = new JMenuItem();
        menuItem.setAction(new icImportSiplogAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S,
                                                       ActionEvent.ALT_MASK));
        menu.add(menuItem);

        menuItem = new JMenuItem();
        menuItem.setAction(new icImportSyslogAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Y,
                                                       ActionEvent.ALT_MASK));
        menu.add(menuItem);

        menu.addSeparator();

        // Add the reload item to the File menu.
        menuItem = new JMenuItem();
        menuItem.setAction(new icReloadAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R,
                                                       ActionEvent.ALT_MASK));
        menu.add(menuItem);

        menu.addSeparator();

        // Add the quite item to the File menu.
        menuItem = new JMenuItem();
        menuItem.setAction(new icQuitAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q,
                                                       ActionEvent.ALT_MASK));
        menu.add(menuItem);

        // Build the Help menu.
        menu = new JMenu("Help");
        menu.setMnemonic(KeyEvent.VK_X);
        menuBar.add(menu);

        // Add the items to the File menu.
        menuItem = new JMenuItem();
        menuItem.setAction(new icAboutAction());
        menu.add(menuItem);
    }

    // A class to reload the last loaded file.
    // Objects of this class do no-op reloads, but subclasses can override
    // method execute() to do useful reloads.
    protected class Reload
    {
        // Method to be overridden by subclasses.
        public void execute()
        {
        }
    }


    protected class icOpenFileAction extends AbstractAction
    {
        public icOpenFileAction()
        {
            super("Open Siptrace File") ;
        }

        protected class ReloadOpenFile extends Reload
        {
            // Store the file name to be reloaded.
            protected String m_fileName;

            public ReloadOpenFile(String fileName)
            {
                m_fileName = fileName;
            }

            // Reload the file.
            public void execute()
            {
                applyData(SipBranchData.getSipBranchDataElements(m_fileName)) ;
            }
        }

        public void actionPerformed(ActionEvent e)
        {
            String strSource = selectFile(getFrame(), "xml",
                                          "SIPViewer XML (pre-rendered)") ;
            if (strSource != null)
            {
                applyData(SipBranchData.getSipBranchDataElements(strSource)) ;
                // Save the reload action.
                m_Reload = new ReloadOpenFile(strSource);
            }
        }
    }

    protected class icImportSiplogAction extends AbstractAction
    {
        public icImportSiplogAction()
        {
            super("Import Siplog") ;
        }

        public void actionPerformed(ActionEvent e)
        {
            String strSource = selectFile(getFrame(), "txt",
                                          "xpressa siplog") ;
            if (strSource != null)
            {
                try
                {
                    applyData(importSipfile(strSource, "siplog2siptrace")) ;
                }
                catch (Exception exception)
                {
                    exception.printStackTrace() ;
                }
            }
        }
    }


    protected class icImportSyslogAction extends AbstractAction
    {
        public icImportSyslogAction()
        {
            super("Import Syslog") ;
        }

        protected class ReloadImportSyslog extends Reload
        {
            // Store the file name to be reloaded.
            protected String m_syslogFileName;

            public ReloadImportSyslog(String syslogFileName)
            {
                m_syslogFileName = syslogFileName;
            }

            // Reload the file.
            public void execute()
            {
                try
                {
                    applyData(importSipfile(m_syslogFileName, "syslog2siptrace")) ;
                }
                catch (Exception exception)
                {
                    exception.printStackTrace() ;
                }
            }
        }

        public void actionPerformed(ActionEvent e)
        {
            String strSource = selectFile(getFrame(), "log",
                                          "OsSysLog formatted file") ;
            if (strSource != null)
            {
                try
                {
                    applyData(importSipfile(strSource, "syslog2siptrace")) ;
                    // Save the reload action.
                    m_Reload = new ReloadImportSyslog(strSource);
                }
                catch (Exception exception)
                {
                    exception.printStackTrace() ;
                }
            }
        }
    }


    protected class icReloadAction extends AbstractAction
    {
        public icReloadAction()
        {
            super("Reload") ;
        }

        public void actionPerformed(ActionEvent e)
        {
            // Execute the reload method.
            m_Reload.execute();
        }
    }


    protected class icQuitAction extends AbstractAction
    {
        public icQuitAction()
        {
            super("Quit") ;
        }

        public void actionPerformed(ActionEvent e)
        {
            System.exit(0);
        }
    }


    protected class icAboutAction extends AbstractAction
    {
        public icAboutAction()
        {
            super("About") ;
        }

        public void actionPerformed(ActionEvent e)
        {
            String strText =
               "SIP Viewer\r\n" +
               "\r\n" +
               "Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.\r\n" +  
               "Contributors retain copyright to elements licensed under a Contributor Agreement.\r\n" +
               "Licensed to the User under the LGPL license.\r\n"
               ;

            JOptionPane.showMessageDialog(getFrame(),
                                          strText,
                                          "About SIP Viewer",
                                          JOptionPane.INFORMATION_MESSAGE);
        }
    }

    protected Vector importSipfile(String importFilename,
                                      String filterName) throws Exception
    {
        File importFile = new File(importFilename);
        String tempFilePrefix = importFile.getName();
        String tempFilename;
        Vector vData = null ;

        File tempFile = File.createTempFile(tempFilePrefix, ".xml");
        tempFilename = tempFile.getAbsolutePath();

        String sysCommand = new String(filterName +
                                       " if=" + importFilename +
                                       " of=" + tempFilename);
        System.out.println("cmd: " + sysCommand);

        Thread.yield() ;
        Process convertProc = Runtime.getRuntime().exec(sysCommand);

        if (tempFilename != null)
        {
            int convertResult = convertProc.waitFor();
            System.out.println("conversion result: " + convertResult);

            if (convertResult == 0)
            {
                // Conversion succeeded.  Read the temporary output file
                // and return.
                vData = SipBranchData.getSipBranchDataElements(tempFilename) ;

                tempFile.delete();
            }
            else
            {
                // We do not delete the temp file so we can figure out what
                // went wrong.
                System.out.println("conversion failed");
            }
        }

        return vData ;
   }
}
