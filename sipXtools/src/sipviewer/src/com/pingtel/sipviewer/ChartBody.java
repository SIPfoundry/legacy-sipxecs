package com.pingtel.sipviewer;

import javax.swing.* ;
import javax.swing.table.* ;
import javax.swing.BorderFactory ;
import javax.swing.border.* ;
import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;


public class ChartBody extends JComponent implements Scrollable
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    protected static final int DEFAULT_MAX_WIDTH_PER_KEY    = 200 ;
    protected static final int DEFAULT_MAX_HEIGHT           = 100 ;
    protected static final int ARROW_HEIGHT                 = 10 ;
    protected static final int ARROW_SHORTEN_WIDTH          = 6 ;


//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    protected SIPChartModel m_model ;
    protected SIPInfoPanel  m_infoPanel ;
    protected int           m_iMouseOver ;
    protected String        m_strMatchBranchId ;
    protected String        m_strMatchTransactionId ;
    protected String        m_strMatchCSeqCallId ;
    protected String        m_strMatchDialogId ;
    protected String        m_strMatchCallId ;

    // Colors for messages

    // The selected message.
    protected Color         m_highlight_color;
    // The same hop-transaction.
    protected Color         m_hop_transaction_color;
    // The a sibling fork of the same hop-transaction.
    protected Color         m_hop_transaction_forked_color;
    // The same end-to-end transaction.
    protected Color         m_transaction_color;
    // A sibling fork of the end-to-end transaction (the to-tag is different).
    protected Color         m_transaction_forked_color;
    // The same dialog.
    protected Color         m_dialog_color;
    // A sibling fork of the dialog (the to-tag is different).
    protected Color         m_dialog_forked_color;
    // Anything else.
    protected Color         m_unhighlighted_color;

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public ChartBody(SIPChartModel model, SIPInfoPanel infoPanel)
    {
        m_model = model ;
        m_iMouseOver = -1 ;
        m_infoPanel = infoPanel ;

        // Initialize the colors.
        // Set how different the "light" and "dark" colors are.
        // 64 seems to give good results.
        int delta = 64;

        // The selected message is red.
        m_highlight_color = Color.red;

        // Other messages in the hop-by-hop transaction are green.
        m_hop_transaction_color =
            new Color(delta, 255, delta); // light green
        m_hop_transaction_forked_color =
            new Color(0, 255-delta, 0); // dark green

        // Other messages in the end-to-end-transaction are blue.
        // Color.blue is too dark.
        int lighten_blue = 128;
        m_transaction_color =
            new Color(delta+lighten_blue, delta+lighten_blue,
                      255);     // light blue
        m_transaction_forked_color =
            new Color(lighten_blue, lighten_blue,
                      Math.min(255-delta+lighten_blue, 255)); // dark blue

        // Other messages in the dialog are yellow.
        m_dialog_color =
            new Color(255,255,delta); // light yellow
        m_dialog_forked_color =
            new Color(255-delta, 255-delta, 0); // dark yellow

        // Unrelated messages are white.
        m_unhighlighted_color = Color.white;

        m_model.addChartModelListener(new icChartModelListener());
        addMouseListener(new icMouseListener());
        addMouseMotionListener(new icMouseMotionListener()) ;
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public void paintEntry(Graphics g, int index)
    {
        Dimension dimSize = getSize() ;             // TODO: Only get once
        Dimension dimRowSize = getMinimumSize() ;   // TODO: Only get once

        int ixOffset = dimSize.width / m_model.getNumKeys() ;
        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor entry = (ChartDescriptor) m_model.getEntryAt(index) ;
            String strBranchId = entry.dataSource.getThisBranchId() ;
            String strTransactionId = entry.dataSource.getTransactionId() ;
            String strCSeqCallId = entry.dataSource.getCSeqCallId() ;
            String strDialogId = entry.dataSource.getDialogId() ;
            String strCallId = entry.dataSource.getCallId() ;
            boolean bEast = true ;

            Rectangle rectAreaText = new Rectangle(
                    ixOffset*entry.sourceColumn + ixOffset/2,
                    (dimRowSize.height)*(index+1),
                    (entry.targetColumn-entry.sourceColumn)*ixOffset,
                    dimRowSize.height-(ARROW_HEIGHT+2)) ;

            Rectangle rectAreaArrow = new Rectangle(
                    ixOffset*entry.sourceColumn + ixOffset/2,
                    (dimRowSize.height)*(index+1),
                    (entry.targetColumn-entry.sourceColumn)*ixOffset,
                    ARROW_HEIGHT) ;

            if (rectAreaArrow.width < 0)
            {
                int oldX = rectAreaArrow.x ;
                rectAreaArrow.x = rectAreaArrow.x + rectAreaArrow.width ;
                rectAreaArrow.width = rectAreaArrow.width * - 1;

                rectAreaText.x = rectAreaArrow.x ;
                rectAreaText.width = rectAreaArrow.width ;

                bEast = false ;
            }

            // Adjust rectangle so that horz lines don't touch the verts.
            rectAreaArrow.x += ARROW_SHORTEN_WIDTH ;
            rectAreaArrow.width -= (ARROW_SHORTEN_WIDTH*2) ;

            // Set the color based on the relationship of this message
            // to the message the mouse is over.
            if (m_iMouseOver == index)
            {
                // The selected message.
                g.setColor(m_highlight_color) ;
            }
            else if ((m_strMatchBranchId != null)
                    && (m_strMatchBranchId.length() > 0)
                    && (m_strMatchBranchId.equals(strBranchId)))
            {
                // The same hop-by-hop transaction as the selected message.
                if ((m_strMatchTransactionId != null)
                    && (m_strMatchTransactionId.length() > 0)
                    && (m_strMatchTransactionId.equals(strTransactionId)))
                {
                    // Same fork ID.
                    g.setColor(m_hop_transaction_color) ;
                }
                else
                {
                    // Different fork ID.
                    g.setColor(m_hop_transaction_forked_color) ;
                }
            }
            else if ((m_strMatchTransactionId != null)
                     && (m_strMatchTransactionId.length() > 0)
                     && (m_strMatchTransactionId.equals(strTransactionId)))
            {
                // The same end-to-end transaction as the selected message,
                // same fork ID.
                g.setColor(m_transaction_color) ;
            }
            else if ((m_strMatchCSeqCallId != null)
                     && (m_strMatchCSeqCallId.length() > 0)
                     && (m_strMatchCSeqCallId.equals(strCSeqCallId)))
            {
                // The same end-to-end transaction as the selected message,
                // different fork ID.
                g.setColor(m_transaction_forked_color) ;
            }
            else if ((m_strMatchDialogId != null)
                     && (m_strMatchDialogId.length() > 0)
                     && (m_strMatchDialogId.equals(strDialogId)))
            {
                // The same dialog as the selected message, same fork ID.
                g.setColor(m_dialog_color) ;
            }
            else if ((m_strMatchCallId != null)
                    && (m_strMatchCallId.length() > 0)
                    && (m_strMatchCallId.equals(strCallId)))
            {
                // The same dialog as the selected message, different fork ID.
                g.setColor(m_dialog_forked_color) ;
            }
            else
            {
                // All other messages.
                g.setColor(m_unhighlighted_color) ;
            }

            if (entry.dataSource.isRequest())
            {
                GUIUtils.drawArrow(g, rectAreaArrow, bEast, null,
                                   GUIUtils.LINE_SOLID) ;
            }
            else
            {
                boolean bIsProvision = false ;

                String strResponseCode = entry.dataSource.getResponseCode() ;
                if ((strResponseCode != null) &&
                    strResponseCode.startsWith("1"))
                {
                    bIsProvision = true ;
                }

                if (bIsProvision)
                {
                    GUIUtils.drawArrow(g, rectAreaArrow, bEast, null,
                                       GUIUtils.LINE_DOTTED) ;
                }
                else
                {
                    GUIUtils.drawArrow(g, rectAreaArrow, bEast, null,
                                       GUIUtils.LINE_DASHED) ;
                }

            }

            int xTextOffset = GUIUtils.calcXOffset(entry.label, g,
                                                   rectAreaText,
                                                   GUIUtils.ALIGN_CENTER) ;
            g.drawString(entry.label, xTextOffset, rectAreaText.y) ;
        }
    }


    public void paint(Graphics g)
    {
        int iEntries = m_model.getSize() ;
        Dimension dimSize = getSize() ;
        Dimension dimRowSize = getMinimumSize() ;
        int iNumKeys = m_model.getNumKeys() ;

        // Paint Background
        // (In case the data panel is smaller than the viewport, we
        // have already set the viewport's background color to black
        // also.)
        g.setColor(Color.black) ;
        g.fillRect(0, 0, dimSize.width, dimSize.height) ;

        // Draw vertical lines
        if ((dimRowSize.width > 0) && (iNumKeys > 0))
        {
            int iOffset = dimSize.width / iNumKeys ;

            for (int i=0; i<iNumKeys; i++)
            {
                g.setColor(Color.yellow);
                g.drawLine((iOffset*i) + iOffset / 2,
                0,
                (iOffset*i) + iOffset / 2,
                dimSize.height) ;
            }
        }

        // Paint all elements
        if ((iEntries > 0) && (dimRowSize.width > 0) && (iNumKeys > 0))
        {
            for (int i=0; i<iEntries; i++)
            {
                paintEntry(g, i) ;
            }
        }
    }


    public Dimension getPreferredSize()
    {
        int width = DEFAULT_MAX_WIDTH_PER_KEY*(m_model.getNumKeys()+1) ;
        int height = DEFAULT_MAX_HEIGHT ;

        Graphics g = getGraphics() ;
        if (g != null)
        {
            Font f = getFont() ;
            if (f != null)
            {
                FontMetrics fm =  g.getFontMetrics(f) ;
                height = fm.getHeight() + ARROW_HEIGHT ;
            }
        }

        height = height * (m_model.getSize() + 1) ;

        return new Dimension(width, height) ;
    }


    public Dimension getMinimumSize()
    {
        int width = DEFAULT_MAX_WIDTH_PER_KEY*(m_model.getNumKeys()+1) ;
        int height = DEFAULT_MAX_HEIGHT ;

        Graphics g = getGraphics() ;
        if (g != null)
        {
            Font f = getFont() ;
            if (f != null)
            {
                FontMetrics fm =  g.getFontMetrics(f) ;
                height = fm.getHeight() + ARROW_HEIGHT ;
            }
        }

        return new Dimension(width, height) ;
    }


    public Dimension getPreferredScrollableViewportSize()
    {
        return getPreferredSize() ;
    }


    public int getScrollableBlockIncrement(Rectangle visibleRect,
                                           int orientation, int direction)
    {
        // The visible area height, minus one arrow spacing.
        return visibleRect.height - getMinimumSize().height ;
    }

    public boolean getScrollableTracksViewportHeight()
    {
        return false ;
    }


    public boolean getScrollableTracksViewportWidth()
    {
        return true ;
    }


    public int getScrollableUnitIncrement(Rectangle visibleRect,
                                          int orientation, int direction)
    {
        // The spacing between arrows.
        return getMinimumSize().height ;
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation
////
    protected int PointToIndex(Point point)
    {
        Dimension size = getMinimumSize() ;
        int index = (point.y - (size.height / 2)) / size.height ;

        if (index >= m_model.getSize())
        {
            index = -1 ;
        }

        return index ;
    }


    protected SipBranchData getSipBranchData(int index)
    {
        SipBranchData objRC = null ;
        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor desc = m_model.getEntryAt(index) ;
            objRC = desc.dataSource ;
        }
        return objRC ;
    }


    protected String getDisplayHeader(int index)
    {
        String strRC = null ;

        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor desc = m_model.getEntryAt(index) ;
            SipBranchData source = desc.dataSource;
            // Java text uses LF only as EOL.
            strRC =
                "Time: " + source.getTimeStamp() + "\n" +
                "Frame: " + source.getFrameId() + "\n" +
                "Source: " + source.getSourceEntity() + "\n" +
                "Dest: " + source.getDestinationEntity() + "\n" +
                "\n";
        }
        return strRC ;
    }


    protected String getFrameId(int index)
    {
        String strRC = null ;

        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor desc = m_model.getEntryAt(index) ;
            SipBranchData source = desc.dataSource;
            strRC = source.getFrameId();
        }
        return strRC ;
    }


    protected String getMessageText(int index)
    {
        String strRC = null ;

        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor desc = m_model.getEntryAt(index) ;
            strRC = desc.dataSource.getMessage() ;
        }
        return strRC ;
    }


    protected void setMouseOver(int iIndex)
    {
        int iOldIndex = m_iMouseOver ;
        m_iMouseOver = iIndex ;
        if (iOldIndex != m_iMouseOver)
        {
            Graphics g = getGraphics() ;
            if (g != null)
            {
                if (iOldIndex != -1)
                {
                    m_strMatchBranchId = null ;
                    m_strMatchTransactionId = null ;
                    m_strMatchCSeqCallId = null ;
                    m_strMatchDialogId = null ;
                    m_strMatchCallId = null ;
                    repaintAllMatching(iOldIndex, false) ;
                }

                if (m_iMouseOver != -1)
                {
                    repaintAllMatching(m_iMouseOver, true) ;
                    m_infoPanel.populate(getSipBranchData(m_iMouseOver)) ;
                }
                else
                {
                    m_infoPanel.clear() ;
                }
            }
        }
    }


    /**
     * Repaint all entries that match the branchId
     */
    protected void repaintAllMatching(int iSourceIndex, boolean bSetMatchId)
    {
        String strCallId = null ;

        if ((iSourceIndex >= 0) && (iSourceIndex < m_model.getSize()))
        {
            ChartDescriptor sourceDesc = m_model.getEntryAt(iSourceIndex) ;
            strCallId = sourceDesc.dataSource.getCallId() ;
            if (bSetMatchId)
            {
                m_strMatchBranchId = sourceDesc.dataSource.getThisBranchId() ;
                m_strMatchTransactionId =
                    sourceDesc.dataSource.getTransactionId() ;
                m_strMatchCSeqCallId = sourceDesc.dataSource.getCSeqCallId() ;
                m_strMatchDialogId = sourceDesc.dataSource.getDialogId() ;
                m_strMatchCallId = strCallId ;
            }

            if ((strCallId != null) && (strCallId.length() > 0))
            {
                Graphics g = getGraphics() ;
                if (g != null)
                {
                    for (int i=0; i<m_model.getSize(); i++)
                    {
                        ChartDescriptor desc = m_model.getEntryAt(i) ;
                        if (strCallId.equals(desc.dataSource.getCallId()))
                        {
                            paintEntry(g, i) ;
                        }
                    }
                }
            }
            else
            {
                Graphics g = getGraphics() ;
                if (g != null)
                {
                    paintEntry(g, iSourceIndex) ;
                }
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Nested classes
////

    protected class icMouseListener implements MouseListener
    {
        // Set this up as a new subclass of JDialog so the dialog itself
        // is accessible inside the close action for dispose() to operate
        // on.
        public class SIPMessageWindow extends JDialog
        {
            // Set up the new window showing text strText.
            public void setUp(String strFrame, String strText) {

                setTitle("SIP Message " + strFrame);
                Container cont = getRootPane() ;
                cont.setLayout(new GridBagLayout());
                GridBagConstraints gbc = new GridBagConstraints() ;

                // Add action for ESC to close the window.
                KeyStroke ks = KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE,
                                                      0);
                Action action = new AbstractAction() {
                        public void actionPerformed(ActionEvent arg0) {
                            dispose();
                        }
                    };
                getRootPane().
                    getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).
                    put(ks, "close");
                getRootPane().
                    getActionMap().
                    put("close", action);

                gbc.weightx = 1.0 ;
                gbc.weighty = 1.0 ;
                gbc.fill = GridBagConstraints.BOTH ;

                JTextArea text_area = new JTextArea(strText);
                text_area.setBorder(BorderFactory.createEmptyBorder(5, 5,
                                                                    5, 5));
                cont.add(new JScrollPane(text_area), gbc) ;

                pack() ;

                // In Java 1.5 we can ensure the window manager sets
                // the location if it wants to.
                //setLocationByPlatform(true);
                setVisible(true) ;
            }
        }

        public void mouseClicked(MouseEvent e)
        {
            SIPMessageWindow window = new SIPMessageWindow() ;
            int iPos = PointToIndex(e.getPoint());
            String strText = getDisplayHeader(iPos) + getMessageText(iPos);
            window.setUp(getFrameId(iPos), strText);
        }

        public void mousePressed(MouseEvent e)
        {

        }

        public void mouseReleased(MouseEvent e)
        {

        }

        public void mouseEntered(MouseEvent e)
        {

        }

        public void mouseExited(MouseEvent e)
        {
            setMouseOver(-1);
        }
    }


    public class icMouseMotionListener implements MouseMotionListener
    {
        public void mouseDragged(MouseEvent e)
        {

        }


        public void mouseMoved(MouseEvent e)
        {
            setMouseOver(PointToIndex(e.getPoint()));
        }
    }


    protected class icChartModelListener implements ChartModelListener
    {
        public void keyAdded(int position)
        {
            repaint() ;
        }


        public void keyDeleted(int position)
        {
            repaint() ;
        }


        public void keyMoved(int oldPosition, int newPosition)
        {
            repaint() ;
        }


        public void entryAdded(int startPosition, int endPosition)
        {
            repaint() ;
        }


        public void entryDeleted(int startPosition, int endPosition)
        {
            repaint() ;
        }
    }
}
