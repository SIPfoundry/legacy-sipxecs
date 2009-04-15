package com.pingtel.sipviewer;

import javax.swing.* ;
import javax.swing.table.* ;
import javax.swing.BorderFactory ;
import javax.swing.border.* ;
import java.awt.* ;
import java.awt.event.* ;
import java.lang.* ;
import java.util.* ;
import java.io.* ;
import java.net.* ;
import java.awt.datatransfer.*;

/**
 *
 */
public class SIPInfoPanel extends Container
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final int DEFAULT_MESSAGES_HEIGHT = 200 ;
    public static final int DEFAULT_BRANCHES_HEIGHT = 100 ;
    public static final int DEFAULT_WIDTH           = 100 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected JTextArea m_taMessage ;
    protected JLabel    m_lblTransactionID ;
    protected JLabel    m_lblFrameID ;
    protected JTextArea m_taBranchIDs;
    protected JLabel    m_lblTimestamp ;

    protected JScrollPane   m_spMessage ;
    protected JScrollPane   m_spBranchIDs ;


//////////////////////////////////////////////////////////////////////////////
// Construction
////

    public SIPInfoPanel()
    {
        createComponents() ;
        layoutComponents() ;
    }

//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public void setMessage(String strMessage)
    {
        m_taMessage.setText(strMessage) ;
        m_taMessage.setCaretPosition(0) ;
    }


    public void setTransactionID(String strTransactionID)
    {
        m_lblTransactionID.setText(strTransactionID) ;

    }


    public void setFrameID(String strFrameID)
    {
        m_lblFrameID.setText(strFrameID) ;
    }

    public void setBranchIDs(Vector vBranchIDs)
    {
        String strBranches = new String() ;

        if (vBranchIDs != null)
        {
            for (int i=0; i<vBranchIDs.size(); i++)
            {
                strBranches += (String) vBranchIDs.elementAt(i) ;
                strBranches += "\r\n" ;
            }
        }

        m_taBranchIDs.setText(strBranches) ;
        m_taBranchIDs.setCaretPosition(0) ;
    }


    public void setTimestamp(String strTimestamp)
    {
        m_lblTimestamp.setText(strTimestamp) ;
    }


    public void clear()
    {
        setMessage(null) ;
        setBranchIDs(null) ;
        setTimestamp(null) ;
        setTransactionID(null) ;
        setFrameID(null) ;
    }


    public void populate(SipBranchData data)
    {
        setMessage(data.getMessage()) ;
        setBranchIDs(data.getBranchIds()) ;
        setTimestamp(data.getTimeStamp()) ;
        setTransactionID(data.getTransactionId()) ;
        setFrameID(data.getFrameId()) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected void createComponents()
    {
        m_taMessage = new JTextArea("<<SIP Message>>") ;
        m_taMessage.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        m_spMessage = new JScrollPane(m_taMessage) ;
        m_lblTransactionID = new JLabel("<<Transaction ID>>") ;
        m_lblFrameID = new JLabel("<<Frame ID>>") ;
        m_taBranchIDs = new JTextArea("<<Branch IDs>>") ;
        m_taBranchIDs.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
        m_spBranchIDs = new JScrollPane(m_taBranchIDs) ;
        m_lblTimestamp = new JLabel("<<Time and Date>>") ;
    }


    protected void layoutComponents()
    {
        JLabel lblBorder = new JLabel() ;
        lblBorder.setBorder(BorderFactory.createTitledBorder("Info"));

        lblBorder.setLayout(new GridBagLayout());

        GridBagConstraints gbcLabel = new GridBagConstraints() ;
        gbcLabel.gridwidth = 1;
        gbcLabel.weightx = 0.0 ;
        gbcLabel.weighty = 0.0 ;
        gbcLabel.fill = GridBagConstraints.NONE ;
        gbcLabel.anchor = GridBagConstraints.NORTHEAST ;
        gbcLabel.insets = new Insets(1, 4, 1, 4) ;

        GridBagConstraints gbcData = new GridBagConstraints() ;
        gbcData.gridwidth = GridBagConstraints.REMAINDER ;
        gbcData.weightx = 1.0 ;
        gbcData.weighty = 0.0 ;
        gbcData.fill = GridBagConstraints.HORIZONTAL ;
        gbcData.anchor = GridBagConstraints.CENTER ;

        lblBorder.add(new JLabel("Time:"), gbcLabel) ;
        lblBorder.add(m_lblTimestamp, gbcData) ;

        lblBorder.add(new JLabel("Trans:"), gbcLabel) ;
        lblBorder.add(m_lblTransactionID, gbcData) ;

        lblBorder.add(new JLabel("Frame:"), gbcLabel) ;
        lblBorder.add(m_lblFrameID, gbcData) ;

        lblBorder.add(new JLabel("Branches:"), gbcLabel) ;
        m_spBranchIDs.setMinimumSize(new Dimension(DEFAULT_WIDTH, DEFAULT_BRANCHES_HEIGHT));
        m_spBranchIDs.setPreferredSize(new Dimension(DEFAULT_WIDTH, DEFAULT_BRANCHES_HEIGHT));

        gbcData.weightx = 1.0 ;
        gbcData.weighty = 1.0 ;
        gbcData.fill = GridBagConstraints.BOTH ;

        lblBorder.add(m_spBranchIDs, gbcData) ;

        setLayout(new GridBagLayout()) ;
        GridBagConstraints gbc = new GridBagConstraints() ;
        gbc.weightx = 1.0 ;
        gbc.weighty = 1.0 ;
        gbc.fill = GridBagConstraints.BOTH ;

        m_spMessage.setMinimumSize(new Dimension(DEFAULT_WIDTH, DEFAULT_MESSAGES_HEIGHT));
        m_spMessage.setPreferredSize(new Dimension(DEFAULT_WIDTH, DEFAULT_MESSAGES_HEIGHT));

        add(lblBorder, gbc) ;
        add(m_spMessage, gbc) ;
    }
}
