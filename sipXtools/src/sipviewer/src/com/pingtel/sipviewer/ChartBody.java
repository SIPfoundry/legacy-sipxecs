package com.pingtel.sipviewer;

import javax.swing.* ;
import javax.swing.table.* ;
import javax.swing.BorderFactory ;
import javax.swing.border.* ;
import java.awt.* ;
import java.awt.event.* ;
import java.util.* ;


public class ChartBody extends JComponent implements Scrollable, ActionListener
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
    
    protected SIPViewerFrame m_frame;
    protected SIPChartModel  m_model ;
    protected SIPInfoPanel   m_infoPanel ;
    protected int            m_iMouseOver ;
    protected int			 iOldIndex ;
    protected String         m_strMatchBranchId ;
    protected String         m_strMatchTransactionId ;
    protected String         m_strMatchCSeqCallId ;
    protected String         m_strMatchDialogId ;
    protected String         m_strMatchCallId ;
    
    // this stores the relative position of a key (column) from the
    // left side of the window
    protected double []      m_key_positions ;
    
    // popup menu used for taking screen shots and other future operations
    protected JPopupMenu m_toolsPopUp;
    protected JMenuItem  menuItem2_1;
    protected JMenuItem  menuItem2_2;

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
    
    // these static variables keep track of both the top and bottom ChartBody
    // object instances
    protected static ChartBody topChart;
    protected static ChartBody bottomChart;
    
    // used to display the dialog with the background colors when assigning a
    // background color to a sip dialog, also the recorded location of the popup
    // menu so that when user decides to set a background the color chooser dialog
    // is displayed close by and not in the upper left corner (which is the default)
    static PopUpUtils.ColorChooserDialog colorChooser = null;
    protected Point						 popUpLocation = new Point();

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    public ChartBody(SIPViewerFrame frame, SIPChartModel model, SIPInfoPanel infoPanel)
    {
        m_frame = frame;
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
        
        // initialize the popup menu
        initPopUp();

        m_model.addChartModelListener(new icChartModelListener());
        addMouseListener(new icMouseListener());
        addMouseMotionListener(new icMouseMotionListener()) ;
        
        // initialize the local key position array to the one initialized in
        // the SIPChartModel class, this array is just used here, it is updated
        // in the ChartHeader class whenever a new key is added
        m_key_positions = m_model.getKeyPositions();
        
        // lets initialize our ChartBody variables, these are used to highlight the
        // messages on both charts if they are visible, might also be used for
        // other things in the future
        if (topChart == null)
        {
        	// the top ChartBody always is instantiated first so if topChart is null
        	// we know that it is being instantiated
        	topChart = this;
        }
        else
        {
        	// top ChartBody is instantiated so we know that this is the second one
        	bottomChart = this;
        }
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////
    public void paintEntry(Graphics g, int index)
    {
        Dimension dimSize = getSize() ;             
        Dimension dimRowSize = getMinimumSize() ;   

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
                    (int) (m_key_positions[entry.sourceColumn] * dimSize.width),
                    (dimRowSize.height)*(index+1),
                    ((int) (m_key_positions[entry.targetColumn] * dimSize.width) -
                     (int) (m_key_positions[entry.sourceColumn] * dimSize.width)),
                    dimRowSize.height-(ARROW_HEIGHT+2)) ;

            Rectangle rectAreaArrow = new Rectangle(
                    (int) (m_key_positions[entry.sourceColumn] * dimSize.width),
                    (dimRowSize.height)*(index+1),
                    ((int) (m_key_positions[entry.targetColumn] * dimSize.width) -
                     (int) (m_key_positions[entry.sourceColumn] * dimSize.width)),
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

            // draw call flow arrows (solid, dotted or dashed)
            if (entry.dataSource.isRequest())
            {
                GUIUtils.drawArrow(g, rectAreaArrow, bEast, null,
                                   entry.backgroundColor, GUIUtils.LINE_SOLID) ;
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
                                       entry.backgroundColor, GUIUtils.LINE_DOTTED) ;
                }
                else
                {
                    GUIUtils.drawArrow(g, rectAreaArrow, bEast, null,
                                       entry.backgroundColor, GUIUtils.LINE_DASHED) ;
                }

            }

            int xTextOffset = GUIUtils.calcXOffset(entry.label, g,
                                                   rectAreaText,
                                                   GUIUtils.ALIGN_CENTER) ;
            
            // if the background color is black do not paint the background since it
            // will break the yellow vertical lines and since background is black by default
            // there is no point painting it
            if (entry.backgroundColor != Color.BLACK)
            {
            	// store current color for now
            	Color colorTmp = g.getColor();
            
            	// setting background color set for this message
            	g.setColor(entry.backgroundColor);
            
            	// get the width of the text area as well as the default x coordinate
            	int width = rectAreaText.width;
            	int xLocation = rectAreaText.x;
            	
            	// arrived through experimentation this is used to make the background fully cover the
            	// label as well as make the label align with regular messages
            	int offset_width = 3;
            	
            	// if we are dealing with messages communicated on the same key the width of the message
            	// will be 0, in this case we want to calculate width of the background, also if the user
            	// moves columns together so that the space between them is less then the length of the message
            	// text we want to make sure that the background is painted behind the whole message label
            	if ((width == 0) || (width < (g.getFontMetrics().charsWidth(entry.label.toCharArray(), 0, entry.label.length()) + offset_width)))
            	{
            		width = g.getFontMetrics().charsWidth(entry.label.toCharArray(), 0, entry.label.length()) + offset_width;
            		xLocation += offset_width;
            	}            
            	else
            	{
            		// when a background is painted between two keys width has to be reduced twice 
            		// the value to align with the arrow backgrounds, again this value was determined
            		// through experimentation
            		width -= (offset_width * 2);
            		xLocation += offset_width;
            	}
            	
            	// fill the background with the color
            	g.fillRect(xLocation, rectAreaText.y - 12, width, rectAreaText.height);

            	// restore original color
            	g.setColor(colorTmp);
            }
            
            // draw the actual text
            g.drawString(entry.label, xTextOffset + 5, rectAreaText.y) ;
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
                g.drawLine((int) (m_key_positions[i] * dimSize.width),
                0,
                (int) (m_key_positions[i] * dimSize.width),
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


    // when user moves the mouse over a significant area
    // this method is called
    protected void setMouseOver(int iIndex)
    {
        iOldIndex = m_iMouseOver ;
        m_iMouseOver = iIndex ;
        
        // do work only if the previous index
        // is different from the current index
        if (iOldIndex != m_iMouseOver)
        {
            Graphics g = getGraphics() ;
            if (g != null)
            {
            	// if the old index was not -1 that means
            	// we had some messages painted, we know that
            	// we are dealing with a new index now so the
            	// previous painted messages have to be cleared
            	// by repainting them white
                if (iOldIndex != -1)
                {               	
                    m_strMatchBranchId = null ;
                    m_strMatchTransactionId = null ;
                    m_strMatchCSeqCallId = null ;
                    m_strMatchDialogId = null ;
                    m_strMatchCallId = null ;
                    repaintAllMatching(iOldIndex, false) ;
                }

                // if the new position is on a new index lets
                // repaint the new messages that correspond to
                // the new index
                if (m_iMouseOver != -1)
                {
                	// repaint messages and set the info panel to the
                	// currently selected message
                    repaintAllMatching(m_iMouseOver, true) ;
                    m_infoPanel.populate(getSipBranchData(m_iMouseOver)) ;
                }
                else
                {
                	// user moved mouse outside of the window so lets
                	// clear out the info panel data
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

    // this method is called when user selects a background color for a sip dialog,
    // it goes through all the messages in a sip dialog and sets their background color
    // that was passed in, the background color is obtained from the color chooser dialog
    protected void setAllMatchingBackgrounds(int iSourceIndex, Color backgroundColor)
    {
        String strCallId = null ;

        if ((iSourceIndex >= 0) && (iSourceIndex < m_model.getSize()))
        {
            ChartDescriptor sourceDesc = m_model.getEntryAt(iSourceIndex) ;
            strCallId = sourceDesc.dataSource.getCallId() ;

            if ((strCallId != null) && (strCallId.length() > 0))
            {
                Graphics g = getGraphics() ;
                if (g != null)
                {
                    for (int i=0; i<m_model.getSize(); i++)
                    {
                        ChartDescriptor desc = m_model.getEntryAt(i) ;
                        
                        // if a message descriptor belongs to sip dialog then
                        // we set its background color
                        if (strCallId.equals(desc.dataSource.getCallId()))
                        {
                            desc.backgroundColor = backgroundColor;
                        }
                    }
                }
            }
            else
            {
            	// any messages that have 0 lenght strCallId we want to handle them
            	// here
                Graphics g = getGraphics() ;
                if (g != null)
                {
                	sourceDesc.backgroundColor = backgroundColor;
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
  
            if (e.isPopupTrigger())
            {
                // if user invokes PopupTrigger (usually a right click)
                // show the popup menu and store the click location
            	popUpLocation.setLocation(e.getX(), e.getY());
                m_toolsPopUp.show(e.getComponent(), e.getX(), e.getY());
            }
            else
            {
                SIPMessageWindow window = new SIPMessageWindow() ;
                int iPos = PointToIndex(e.getPoint());
                String strText = getDisplayHeader(iPos) + getMessageText(iPos);
                window.setUp(getFrameId(iPos), strText);
            }
        }

        public void mousePressed(MouseEvent e)
        {
            // on different OSes popup triggers are different, that is
            // why we much check for popup trigger on mouseClicked and also
            // on mousePressed and mouseRelease, to cover all possibilities
            if (e.isPopupTrigger())
            {
                // show popup menu and store the click location
            	popUpLocation.setLocation(e.getXOnScreen(), e.getYOnScreen());
                m_toolsPopUp.show(e.getComponent(), e.getX(), e.getY());
            }
        }

        public void mouseReleased(MouseEvent e)
        {
            if (e.isPopupTrigger())
            {
                // show popup menu and store the click location
            	popUpLocation.setLocation(e.getX(), e.getY());
                m_toolsPopUp.show(e.getComponent(), e.getX(), e.getY());
            }
        }

        public void mouseEntered(MouseEvent e)
        {

        }

        public void mouseExited(MouseEvent e)
        {
        	// repaint only if the popup menu is not displayed, if it is then we
        	// don't want to repaint the background screen since it paints over the
        	// popup menu
        	if (!topChart.m_toolsPopUp.isShowing() && !bottomChart.m_toolsPopUp.isShowing())
        	{
        		topChart.setMouseOver(-1);
            
        		// if the bottom chart is also visible (user is working in
        		// split screen mode) then also repaint messages on that
        		// ChartBody
        		if (m_frame.getPaneVisibility(SIPViewerFrame.bottomPaneID))
        			bottomChart.setMouseOver(-1);
        	}
        }
    }


    public class icMouseMotionListener implements MouseMotionListener
    {
        public void mouseDragged(MouseEvent e)
        {

        }


        public void mouseMoved(MouseEvent e)
        {
        	// repaint only if the popup menu is not displayed, if it is then we
        	// don't want to repaint the background screen since it paints over the
        	// popup menu
        	if (!topChart.m_toolsPopUp.isShowing() && !bottomChart.m_toolsPopUp.isShowing())
        	{
        		int verticalIndex = PointToIndex(e.getPoint());
        	
        		// call the setMouseOver for the top chart to repaint
        		// the highlighted messages
        		topChart.setMouseOver(verticalIndex);
            
        		// if the bottom chart is also visible (user is working in
        		// split screen mode) then also repaint messages on that
        		// ChartBody
        		if (m_frame.getPaneVisibility(SIPViewerFrame.bottomPaneID))
        			bottomChart.setMouseOver(verticalIndex);
        	}      
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
    
    // initalize the popup menu for this panel
    protected void initPopUp ()
    {       
        JMenuItem menuItem;
    
        m_toolsPopUp = new JPopupMenu();
        menuItem = new JMenuItem(PopUpUtils.Item1);
        menuItem.addActionListener(this);
        m_toolsPopUp.add(menuItem);
        
        m_toolsPopUp.addSeparator();

        // we keep track of the single and double screen
        // modes so that we can make the popup menu choices
        // different depending on which mode we're in
        menuItem2_1 = new JMenuItem(PopUpUtils.Item2_1);
        menuItem2_1.addActionListener(this);
        m_toolsPopUp.add(menuItem2_1);
        
        menuItem2_2 = new JMenuItem(PopUpUtils.Item2_2);
        menuItem2_2.addActionListener(this);        
        m_toolsPopUp.add(menuItem2_2);
        menuItem2_2.setVisible(false);
        
        m_toolsPopUp.addSeparator();
        
        menuItem = new JMenuItem(PopUpUtils.Item3);
        menuItem.addActionListener(this);
        m_toolsPopUp.add(menuItem);
    }
    
    
    @Override
    public void actionPerformed(ActionEvent arg0) 
    {       	
    	// doing a screen capture
    	if (arg0.getActionCommand().compareTo(PopUpUtils.Item1) == 0)
    	{
    		// thread that will do the actual screen-shot
    		Thread popupThread;
    
    		popupThread = new Thread ( new Runnable() {        
    			public void run () 
    			{               
    				try 
    				{
    					// wait for 2 seconds before taking snapshot
    					Thread.sleep(2000);
       
    					// calling our popup menu utilities to 
    					// capture the screen
    					PopUpUtils.captureScreen(m_frame);
    				}
    				catch (InterruptedException ie) {}
    			}});
    
    		popupThread.start();
    	}
    	else if (arg0.getActionCommand().compareTo(PopUpUtils.Item2_1) == 0)
    	{
    		// switching to a double screen mode
    		m_frame.setSecondPaneVisiblity(true);
    	}
    	else if (arg0.getActionCommand().compareTo(PopUpUtils.Item2_2) == 0)
    	{
    		// switching to a single screen mode
    		m_frame.setSecondPaneVisiblity(false);
    	}
    	else if (arg0.getActionCommand().compareTo(PopUpUtils.Item3) == 0)
    	{        	
    		// if the dialog has not yet been initialized then get an instance
    		// we only do this once per execution of sipviewer and then reuse it
    		// if user invokes it again
    		if (colorChooser == null)    		
    			colorChooser = new PopUpUtils.ColorChooserDialog(m_frame);
    		
    		// reset the selected color held by the dialog
    		colorChooser.resetSelectedColor();
    		
    		// set the location of the colorChooser dialog
    		colorChooser.setLocation(popUpLocation);
    		
    		// show the dialog to the user
    		colorChooser.setVisible(true);
    		    	    	
    		// when dialog is dismissed we continue, if user picked a color we
    		// use it to set all the backgrounds for messages that are part
    		// of the immediate sip dialog
    		if (colorChooser.getSelectedColor() != null)
    		{
    			// set the background for all the significant messages, each
    			// descriptor has its background color value assigned
    			setAllMatchingBackgrounds(iOldIndex, colorChooser.getSelectedColor());
    		}
        	        	
        	// trigger a repaint of the top chart and bottom chart if it is visible
        	topChart.repaint();
        	
        	// because the chart descriptors are shared between ChartBody instances
        	// setting the color for each descriptor sets it automatically for both
        	// ChartBody instances
        	if (m_frame.getPaneVisibility(SIPViewerFrame.bottomPaneID))
    			bottomChart.repaint();
    	}
    }
    
    // used to adjust the popup menu after the screen is split/unsplit
    public void splitScreenPopUpChange(boolean split)
    {
    	if (split)
    	{
    		// adjusting popup menu options
    		menuItem2_1.setVisible(false);
    		menuItem2_2.setVisible(true);
    	}
    	else
    	{
    		// adjusting popup menu options
    		menuItem2_1.setVisible(true);
    		menuItem2_2.setVisible(false);
    	}
    }
}
