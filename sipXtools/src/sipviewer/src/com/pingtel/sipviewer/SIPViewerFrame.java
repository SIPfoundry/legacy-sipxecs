package com.pingtel.sipviewer;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.net.URL;
import java.util.Vector;

import javax.swing.AbstractAction;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.KeyStroke;

public class SIPViewerFrame extends JFrame
{
    // //////////////////////////////////////////////////////////////////////
    // Constants
    // //

    // //////////////////////////////////////////////////////////////////////
    // Attributes
    // //
    // m_modal is used to communicate events between the Frame and the header
    // and body
    // m_body is the main area where the columns and messages are displayed
    // m_bodySecond is a duplicate of m_body to provide split screen
    // functionality
    // m_header is the top of the message chart and controls columns
    // m_scrollPane is the container for m_body
    // m_scrollPaneSecond is the container for m_bodySecond
    // m_infoPanel is the bottom part of the sipviewer below the message chart

    protected SIPChartModel m_model;
    protected ChartBody m_body;
    protected ChartBody m_bodySecond;
    protected ChartHeader m_header;
    protected JScrollPane m_scrollPane;
    protected JScrollPane m_scrollPaneSecond;
    protected SIPInfoPanel m_infoPanel;
    protected String m_fileChooserDir;
    protected boolean m_sortBranchNodes;
    // Initialize with a trivial reload object so Reload does nothing.
    protected Reload m_Reload = new Reload();

    // used to identify which pane (top or bottom) is being queried/
    // operated on
    protected static int topPaneID = 0;
    protected static int bottomPaneID = 1;

    public SIPViewerFrame(boolean createMenu) {
        super("sipviewer");

        this.setSize(800, 600);
        m_sortBranchNodes = false;
        this.addWindowListener(new icWindowAdapter());

        createComponents();
        layoutComponents();
        if (createMenu)
            initMenu();
    }

    /*
     * 
     */
    public void applyAliasesFile(String strAliasesFile)
    {
        try
        {
            FileReader fr = new FileReader(strAliasesFile);

            BufferedReader reader = new BufferedReader(fr);
            String strLine = reader.readLine();
            while (strLine != null)
            {
                int pos = strLine.indexOf("=");
                if (pos > 0)
                {
                    String strValue = strLine.substring(0, pos);
                    String strKey = strLine.substring(pos + 1);

                    strKey = strKey.trim();
                    strValue = strValue.trim();

                    // System.out.println("AddAlias: " + strValue + " -> " +
                    // strKey) ;

                    m_model.removeKey(strValue);
                    m_model.addKeyAlias(strKey, strValue);
                    m_model.reindexData();
                }

                strLine = reader.readLine();
            }

            reader.close();
            fr.close();
        } catch (Exception e)
        {
            System.out.println("Unable to apply aliases file: " + strAliasesFile);

            JOptionPane.showConfirmDialog(null, "Unable to apply aliases file: " + strAliasesFile,
                    "Error", JOptionPane.DEFAULT_OPTION, JOptionPane.ERROR_MESSAGE, null);
            e.printStackTrace();
            this.dispose();
        }
    }

    public void applySourceFile(URL url) throws Exception
    {

        try
        {

            // whenever loading a new file or reloading a file make sure to
            // reset the the pane visibility to single screen, if file
            // contains sipviewer meta info and pane is split then it will
            // be split in the SipViewerMetaData methods
            setSecondPaneVisibility(false);
            
            // resets the values of the count of messages that each key
            // has against itself, in other words, each column has a
            // message originating from it to going to it, each of 
            // these origins and destinations against a column is
            // summed together, this methods resets this value 0
            // because we are reloading the file
            m_model.resetKeyUsageValues();

            // Save the reload action.
            m_Reload = new ReloadOpenFile(url.toString());
        } catch (Exception ex)
        {
            System.err.println("Unexpected exception ");
            ex.printStackTrace();
        }
    }

    public void applySourceFile(String strSourceFile) throws Exception
    {

        try
        {

            // whenever loading a new file or reloading a file make sure to
            // reset the the pane visibility to single screen, if file
            // contains sipviewer meta info and pane is split then it will
            // be split in the SipViewerMetaData methods
            setSecondPaneVisibility(false);
            
            // resets the values of the count of messages that each key
            // has against itself, in other words, each column has a
            // message originating from it to going to it, each of 
            // these origins and destinations against a column is
            // summed together, this methods resets this value 0
            // because we are reloading the file
            m_model.resetKeyUsageValues();

            // Save the reload action.
            m_Reload = new ReloadOpenFile(strSourceFile);
        } catch (Exception ex)
        {
            System.err.println("Unexpected exception ");
            ex.printStackTrace();
        }
    }

    // take the parsed XML data that was converted to SIPBranchData objects and
    // add them to the ChartDescriptor objects
    protected void applyData(Vector vData)
    {
        m_model.clear();

        if (m_sortBranchNodes)
        {
            // Do some magic to adjust for the case where the response
            // shows up just before the request.
            SipBranchData current = null;
            SipBranchData previous = null;
            String strCurrentBranchId;
            String strPreviousBranchId;
            for (int i = vData.size() - 1; i > 0; i--)
            {
                current = (SipBranchData) vData.elementAt(i);
                strCurrentBranchId = current.getThisBranchId();

                previous = (SipBranchData) vData.elementAt(i - 1);
                strPreviousBranchId = current.getThisBranchId();

                if ((strCurrentBranchId != null) && (strCurrentBranchId.length() > 0)
                        && (strPreviousBranchId != null) && (strPreviousBranchId.length() > 0)
                        && (strCurrentBranchId.equals(strPreviousBranchId))
                        && (current.isRequest()) && (!previous.isRequest())
                        && (current.getMethod() != "ack"))
                {
                    vData.set(i, previous);
                    vData.set(i - 1, current);

                    System.out.println("Req/Resp swapped for branchID: " + strCurrentBranchId);
                }
            }
        }

        SipBranchData data = null;

        // loop over all the data elements and add them
        // to the model
        for (int i = 0; i < vData.size(); i++)
        {
            data = (SipBranchData) vData.elementAt(i);

            // pass in the for loop index since it is used
            // to set the displayIndex of each message, this
            // displayIndex is used to determine where the
            // message is displayed vertically and weather
            // it is visible or not (displayIndex of
            // -1 is invisible)
            addEntryToModel(data, i);
        }

        // Reset the scroll bar to the top.
        JScrollBar scroll = m_scrollPane.getVerticalScrollBar();
        scroll.setValue(scroll.getMinimum());

        // Revalidate to make sure the drawing pane is resized to match the
        // data.
        m_body.revalidate();
    }

    protected void addEntryToModel(SipBranchData data, int displayIndex)
    {
        String strSourceAliases = null;
        String strTargetAliases = null;
        String strLabel = "Error";
        String strSource;
        String strTarget;

        if (data.getSourceEntity() != null)
        {
            strSource = data.getSourceEntity();
            strSourceAliases = data.getSourceAddress();
        }
        else if (data.getSourceAddress() != null)
        {
            strSource = data.getSourceAddress();
        }
        else
            strSource = "Unknown";

        if (data.getDestinationEntity() != null)
        {
            strTarget = data.getDestinationEntity();
            strTargetAliases = data.getDestinationEntity();
        }
        else if (data.getDestinationAddress() != null)
        {
            strTarget = data.getDestinationAddress();
        }
        else
            strTarget = "Unknown";

        if (data.isRequest())
        {
            strLabel = data.getMethod();
        }
        else
        {
            strLabel = data.getResponseCode() + " " + data.getResponseText();
        }

        m_model.addEntry(strSource, strTarget, strLabel, data, displayIndex);
        if (strSourceAliases != null)
            m_model.addKeyAlias(strSource, strSourceAliases);
        if (strTargetAliases != null)
            m_model.addKeyAlias(strTarget, strTargetAliases);
    }

    protected void createComponents()
    {
        m_model = new SIPChartModel();
        m_infoPanel = new SIPInfoPanel();

        m_body = new ChartBody(this, m_model, m_infoPanel);
        m_bodySecond = new ChartBody(this, m_model, m_infoPanel);
        m_header = new ChartHeader(m_model, m_body, m_bodySecond, m_infoPanel);

        m_scrollPane = new JScrollPane(m_body, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        m_scrollPane.setColumnHeaderView(m_header);

        // Make the viewport's background color black, in case the displayed
        // data panel is smaller than the viewport.
        m_scrollPane.getViewport().setBackground(Color.black);

        // setting the default scroll bar size since starting from command line
        // assigns initial value of 100, if meta data was provided then setting
        // the relative position will be wrong
        m_scrollPane.getVerticalScrollBar().setMaximum(1584);

        m_scrollPaneSecond = new JScrollPane(m_bodySecond, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

        // Make the viewport's background color black, in case the displayed
        // data panel is smaller than the viewport.
        m_scrollPaneSecond.getViewport().setBackground(Color.black);

        // setting the default scroll bar size since starting from command line
        // assigns initial value of 100, if meta data was provided then setting
        // the relative position will be wrong
        m_scrollPaneSecond.getVerticalScrollBar().setMaximum(1584);

        // by default make the second pane invisible
        m_scrollPaneSecond.setVisible(false);
    }

    protected void layoutComponents()
    {
        Container rootPane = this.getContentPane();

        Container tempCont = new Container();
        tempCont.setLayout(new GridBagLayout());

        GridBagConstraints gbc = new GridBagConstraints();
        gbc.weightx = 1.0;
        gbc.weighty = 1.0;
        gbc.fill = GridBagConstraints.BOTH;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        tempCont.add(m_scrollPane, gbc);

        gbc.weightx = 1.0;
        gbc.weighty = 1.0;
        gbc.fill = GridBagConstraints.BOTH;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        tempCont.add(m_scrollPaneSecond, gbc);

        gbc.weightx = 1.0;
        gbc.weighty = 0.0;
        gbc.gridwidth = 1;
        gbc.fill = GridBagConstraints.HORIZONTAL;
        tempCont.add(m_infoPanel, gbc);

        rootPane.add(tempCont, BorderLayout.CENTER);
    }

    protected JFrame getFrame()
    {
        return this;
    }

    protected class icWindowAdapter extends WindowAdapter
    {
        public void windowOpened(WindowEvent e)
        {

        }

        public void windowClosing(WindowEvent e)
        {

            // lets close this frame
            SIPViewerFrame.this.dispose();
            System.exit(0);
        }
    }

    protected String selectFile(JFrame parent, String strFilter, String strFilterDesc)
    {
        String fileName = null;
        if (m_fileChooserDir == null || m_fileChooserDir.length() < 1)
        {
            m_fileChooserDir = ".";
        }
        System.out.println("chooser dir: " + m_fileChooserDir);
        JFileChooser chooser = new JFileChooser(m_fileChooserDir);

        ExampleFileFilter filter = new ExampleFileFilter();
        filter.addExtension(strFilter);
        filter.setDescription(strFilterDesc);
        chooser.setFileFilter(filter);

        int returnVal = chooser.showOpenDialog(parent);
        if (returnVal == JFileChooser.APPROVE_OPTION)
        {
            File file = chooser.getSelectedFile();
            fileName = file.getAbsolutePath();

            // Set the default directory for next time
            m_fileChooserDir = file.getParentFile().getAbsolutePath();
            System.out.println("saving chooser dir: " + m_fileChooserDir);

            String fileNameNoPath = file.getName();
            this.setTitle("SIP Viewer - " + fileNameNoPath);
        }
        return (fileName);
    }

    protected void initMenu()
    {
        JMenu menu;
        JMenuItem menuItem;

        // Create the menu bar.
        JMenuBar menuBar = new JMenuBar();
        this.setJMenuBar(menuBar);

        // Build the File menu.
        menu = new JMenu("File");
        // menu.setMnemonic(KeyEvent.VK_A);
        menuBar.add(menu);

        // Add the load-file items to the File menu.
        menuItem = new JMenuItem();
        menuItem.setAction(new icOpenFileAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F, ActionEvent.ALT_MASK));
        menu.add(menuItem);

        menuItem = new JMenuItem();
        menuItem.setAction(new icImportSyslogAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Y, ActionEvent.ALT_MASK));
        menu.add(menuItem);

        menu.addSeparator();

        menuItem = new JMenuItem();
        menuItem.setAction(new icSaveAsAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, ActionEvent.ALT_MASK));
        menu.add(menuItem);

        menu.addSeparator();

        // Add the reload item to the File menu.
        menuItem = new JMenuItem();
        menuItem.setAction(new icReloadAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_R, ActionEvent.ALT_MASK));
        menu.add(menuItem);

        menu.addSeparator();

        // Add the quite item to the File menu.
        menuItem = new JMenuItem();
        menuItem.setAction(new icQuitAction());
        menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q, ActionEvent.ALT_MASK));
        menu.add(menuItem);

        // Build the Options menu.
        menu = new JMenu("Options");
        menu.setMnemonic(KeyEvent.VK_X);
        menuBar.add(menu);

        // Add the split/single screen mode to the options menu
        menuItem = new JMenuItem();
        menuItem.setAction(new icScreenModeAction());
        menu.add(menuItem);

        menu.addSeparator();

        // Add the show all dialogs option to the options menu
        menuItem = new JMenuItem();
        menuItem.setAction(new icShowAllDialogsAction());
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

        public String getOpenFile()
        {
            return null;
        }

        // Method to be overridden by subclasses.
        public void execute()
        {
        }
    }

    private class ReloadOpenFile extends Reload
    {
        // Store the file name to be reloaded.
        protected String m_fileName = null;

        // save the file name which is later used in the
        // execute() method when/if the file is reloaded
        public ReloadOpenFile(String fileName) {
            m_fileName = fileName;
        }

        public String getOpenFile()
        {
            return m_fileName;
        }

        // Reload the file, this is used when user reloads the file
        // virtually identical to the initial load
        public void execute()
        {

            try
            {

                // whenever loading a new file or reloading a file make sure to
                // reset the the pane visibility to single screen, if file
                // contains sipviewer meta info and pane is split then it will
                // be split in the SipViewerMetaData methods
                setSecondPaneVisibility(false);
                
                // resets the values of the count of messages that each key
                // has against itself, in other words, each column has a
                // message originating from it to going to it, each of 
                // these origins and destinations against a column is
                // summed together, this methods resets this value 0
                // because we are reloading the file
                m_model.resetKeyUsageValues();

                applyData(SipBranchData.getSipBranchDataElements(new File(m_fileName).toURL()));

                // now that we got the data lets see if there is any sipviewer
                // config data stored in this files as
                if (SipBranchData.nodeContainer != null)
                {
                    // lets see if we can get some sipviewer meta data from the
                    // file
                    SipViewerMetaData.setSipViewerMetaData(m_header, m_model, SIPViewerFrame.this,
                            m_scrollPane, m_scrollPaneSecond);
                }

            } catch (Exception ex)
            {
                System.err.println("Bad File" + m_fileName);
                ex.printStackTrace();
            }
        }
    }

    protected class icOpenFileAction extends AbstractAction
    {
        public icOpenFileAction() {
            super("Open Siptrace File");
        }

        public void actionPerformed(ActionEvent e)
        {
            String strSource = selectFile(getFrame(), "xml", "SIPViewer XML (pre-rendered)");
            if (strSource != null)
            {
                try
                {

                    // whenever loading a new file or reloading a file make sure
                    // to reset the the pane visibility to single screen, if
                    // file contains sipviewer meta info and pane is split then
                    // it will be split in the SipViewerMetaData methods
                    setSecondPaneVisibility(false);
                    
                    // resets the values of the count of messages that each key
                    // has against itself, in other words, each column has a
                    // message originating from it to going to it, each of 
                    // these origins and destinations against a column is
                    // summed together, this methods resets this value 0
                    // because we are reloading the file
                    m_model.resetKeyUsageValues();

                    // get the data from the file, a lot of stuff is actually
                    // going on on this line the file is parsed to get the XML
                    // data, then the XML data is parsed to extract each SIP
                    // message in XML format, lastly, each message is parsed
                    // again to convert it to SIPBranchData class object and
                    // finally its added to the ChartDescriptor
                    applyData(SipBranchData.getSipBranchDataElements(new File(strSource).toURL()));

                    // now that we got the data lets see if there is any
                    // sipviewer config data stored in this files as
                    if (SipBranchData.nodeContainer != null)
                    {
                        // lets see if we can get some sipviewer meta data from
                        // the file
                        SipViewerMetaData.setSipViewerMetaData(m_header, m_model,
                                SIPViewerFrame.this, m_scrollPane, m_scrollPaneSecond);
                    }

                    // Save the reload action.
                    m_Reload = new ReloadOpenFile(strSource);
                } catch (Exception ex)
                {
                    System.err.println("Unexpected exception ");
                    ex.printStackTrace();
                }

                SIPViewerFrame.this.validate();
            }
        }
    }

    protected class icSaveAsAction extends AbstractAction
    {
        public icSaveAsAction() {
            super("Save As..");
        }

        public void actionPerformed(ActionEvent e)
        {

            if ((m_Reload.getOpenFile() != null) && (SipBranchData.nodeContainer != null))
            {
                SipViewerMetaData.saveSipViewerMetaData(SIPViewerFrame.this,
                        m_Reload.getOpenFile(), m_header, m_model, SIPViewerFrame.this,
                        m_scrollPane, m_scrollPaneSecond);
            }
        }
    }

    protected class icImportSyslogAction extends AbstractAction
    {
        public icImportSyslogAction() {
            super("Import Syslog");
        }

        protected class ReloadImportSyslog extends Reload
        {
            // Store the file name to be reloaded.
            protected String m_syslogFileName;

            public ReloadImportSyslog(String syslogFileName) {
                m_syslogFileName = syslogFileName;
            }

            // Reload the file.
            public void execute()
            {
                try
                {
                    applyData(importSipfile(m_syslogFileName, "syslog2siptrace"));
                } catch (Exception exception)
                {
                    exception.printStackTrace();
                }
            }
        }

        public void actionPerformed(ActionEvent e)
        {
            String strSource = selectFile(getFrame(), "log", "OsSysLog formatted file");
            if (strSource != null)
            {
                try
                {
                    applyData(importSipfile(strSource, "syslog2siptrace"));
                    // Save the reload action.
                    m_Reload = new ReloadImportSyslog(strSource);
                } catch (Exception exception)
                {
                    exception.printStackTrace();
                }
            }
        }
    }

    protected class icReloadAction extends AbstractAction
    {
        public icReloadAction() {
            super("Reload");
        }

        public void actionPerformed(ActionEvent e)
        {
            // Execute the reload method.
            m_Reload.execute();
        }
    }

    protected class icQuitAction extends AbstractAction
    {
        public icQuitAction() {
            super("Quit");
        }

        public void actionPerformed(ActionEvent e)
        {
            SIPViewerFrame.this.dispose();
            System.exit(0);
        }
    }

    // handles changing the screen mode to single or split
    protected class icScreenModeAction extends AbstractAction
    {
        public icScreenModeAction() {
            super("Toggle Scren Mode (Single/Split)");
        }

        public void actionPerformed(ActionEvent e)
        {

            // toggle the second pane visibility
            if (getPaneVisibility(SIPViewerFrame.bottomPaneID))
            {
                setSecondPaneVisibility(false);
            }
            else
            {
                setSecondPaneVisibility(true);
            }
        }
    }

    // handles resetting all sip dialogues invisibility to visible
    protected class icShowAllDialogsAction extends AbstractAction
    {
        public icShowAllDialogsAction() {
            super("Show All Dialogs");
        }

        public void actionPerformed(ActionEvent e)
        {
            m_body.showAllDialogs();

        }
    }

    protected class icAboutAction extends AbstractAction
    {
        public icAboutAction() {
            super("About");
        }

        public void actionPerformed(ActionEvent e)
        {
            String strText = "SIP Viewer\r\n"
                    + "\r\n"
                    + "Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.\r\n"
                    + "Contributors retain copyright to elements licensed under a Contributor Agreement.\r\n"
                    + "Licensed to the User under the LGPL license.\r\n";

            JOptionPane.showMessageDialog(getFrame(), strText, "About SIP Viewer",
                    JOptionPane.INFORMATION_MESSAGE);
        }
    }

    protected Vector importSipfile(String importFilename, String filterName) throws Exception
    {
        File importFile = new File(importFilename);
        String tempFilePrefix = importFile.getName();
        String tempFilename;
        Vector vData = null;

        File tempFile = File.createTempFile(tempFilePrefix, ".xml");
        tempFilename = tempFile.getAbsolutePath();

        String sysCommand = new String(filterName + " if=" + importFilename + " of=" + tempFilename);
        System.out.println("cmd: " + sysCommand);

        Thread.yield();
        Process convertProc = Runtime.getRuntime().exec(sysCommand);

        if (tempFilename != null)
        {
            int convertResult = convertProc.waitFor();
            System.out.println("conversion result: " + convertResult);

            if (convertResult == 0)
            {
                // Conversion succeeded. Read the temporary output file
                // and return.
                try
                {
                    vData = SipBranchData.getSipBranchDataElements(new File(tempFilename).toURL());

                    tempFile.delete();
                } catch (Exception ex)
                {
                    System.err.println("Bad file " + tempFilename);
                    ex.printStackTrace();
                }
            }
            else
            {
                // We do not delete the temp file so we can figure out what
                // went wrong.
                System.out.println("conversion failed");
            }
        }

        return vData;
    }

    // this is called whenever user toggles between
    // single v.s. double scroll panes
    public void setSecondPaneVisibility(boolean visible)
    {
        // set the visibility of the second pane
        m_scrollPaneSecond.setVisible(visible);

        // make the frame re-do its layout
        SIPViewerFrame.this.validate();
    }

    // this returns weather the pane that ChartBody instance is
    // interested in is visible or not
    public boolean getPaneVisibility(int paneID)
    {
        if (paneID == SIPViewerFrame.topPaneID)
        {
            return m_scrollPane.isVisible();
        }
        else if (paneID == SIPViewerFrame.bottomPaneID)
        {
            return m_scrollPaneSecond.isVisible();
        }

        return false;
    }
}
