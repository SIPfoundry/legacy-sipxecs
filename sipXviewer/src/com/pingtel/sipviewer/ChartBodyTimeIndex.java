/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package com.pingtel.sipviewer;

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;

import javax.swing.JComponent;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.Scrollable;

public class ChartBodyTimeIndex extends JComponent implements Scrollable, ActionListener
{
    // ////////////////////////////////////////////////////////////////////////////
    // Constants
    // //
    protected static final int DEFAULT_MAX_WIDTH_PER_KEY = ChartBody.DEFAULT_MAX_WIDTH_PER_KEY;
    protected static final int DEFAULT_MAX_HEIGHT = ChartBody.DEFAULT_MAX_HEIGHT;
    protected static final int ARROW_HEIGHT = 10;

    // ////////////////////////////////////////////////////////////////////////////
    // Attributes
    // //

    protected SIPViewerFrame m_frame;
    protected SIPChartModel m_model;
    protected int m_iMouseOver;
    protected int iOldIndex;

    // popup menu used for taking screen shots and other future operations
    protected JPopupMenu m_toolsPopUp;

    // Colors for messages
    // The selected message.
    protected Color m_highlight_color;

    // Anything else.
    protected Color m_unhighlighted_color;

    // ////////////////////////////////////////////////////////////////////////////
    // Construction
    // //
    public ChartBodyTimeIndex(SIPViewerFrame frame, SIPChartModel model) {
        m_frame = frame;
        m_model = model;
        m_iMouseOver = -1;

        // The selected message is red.
        m_highlight_color = Color.red;

        // Unrelated messages are white.
        m_unhighlighted_color = Color.white;

        // initialize the popup menu
        initPopUp();

        m_model.addChartModelListener(new icChartModelListener());
        addMouseListener(new icMouseListener());
        addMouseMotionListener(new icMouseMotionListener());
    }

    // method used to paint the background alternating dark gray rectangles
    public void paintBackgroundRow(Graphics g, int index)
    {
        Dimension dimSize = getSize();
        Dimension dimRowSize = getMinimumSize();

        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor entry = (ChartDescriptor) m_model.getEntryAt(index);

            // if display index has been set to < 0 that means user decided not
            // to display the message
            if (entry.displayIndex < 0)
                return;

            Rectangle backgroundArea = new Rectangle(0, (dimRowSize.height)
                    * (entry.displayIndex + 1) - 12, dimSize.width, ARROW_HEIGHT + 14);

            if (entry.backgroundColor != Color.BLACK)
            {
                GUIUtils.paintBackgroundRowCustomColor(g, backgroundArea, entry.backgroundColor);
            }
            else
            {
                // paint every alternate row
                if (entry.displayIndex % 2 == 0)
                    GUIUtils.paintBackgroundRow(g, backgroundArea);
            }
        }
    }

    // ////////////////////////////////////////////////////////////////////////////
    // Public Methods
    // //
    public void paintEntry(Graphics g, int index)
    {

        Dimension dimRowSize = getMinimumSize();

        if ((index >= 0) && (index < m_model.getSize()))
        {
            ChartDescriptor entry = (ChartDescriptor) m_model.getEntryAt(index);

            // if display index has been set to -1 that means user decided not
            // to display the message
            if (entry.displayIndex < 0)
                return;

            // temporary entry variable used to work with the current
            // message that is moused over
            ChartDescriptor tmpEntry = null;

            if (m_iMouseOver != -1)
            {
                tmpEntry = (ChartDescriptor) m_model.getEntryAt(m_iMouseOver);
            }

            // if we are over a new time index entry then lets set the color
            // to the highlight color
            if ((tmpEntry != null) && (tmpEntry.displayIndex == entry.displayIndex))
            {
                // we have a valid entry at the specified index and the display
                // index of the current selected message is equal to the display
                // index of the message being painted
                g.setColor(m_highlight_color);
            }
            else
            {
                // all other time indexes get the plain white display
                g.setColor(m_unhighlighted_color);
            }

            Font m_font = getFont();

            FontMetrics m_fontMetrics = g.getFontMetrics(m_font);          
            
            // if we are dealing with the key index entry AND user is
            // currently not over it then repaint it with highlight
            // color, the only other exception is when user JUST set
            // the key index then we want to repaint it
            if ((index == PopUpUtils.keyIndex))// && (g.getColor() == m_unhighlighted_color))// || keyIndexChanged))
            {
                // we want to make sure that keyindex really stands out
                // so we're painting it yellow and in-casing it in the
                // >>value<< characters, but we only want to do this
                // when user is not over the entry so that it will
                // still paint red when its hovered over
                if ((g.getColor() == m_unhighlighted_color))
                    g.setColor(Color.YELLOW);

                // we display the message number index here (eg. 1, 2, 3, ...,
                // n)
                g.drawString(String.valueOf(entry.displayIndex)
                        + String.valueOf(Character.toChars(187)), 29 - m_fontMetrics
                        .stringWidth(String.valueOf(entry.displayIndex)), (dimRowSize.height)
                        * (entry.displayIndex + 1) + 4);

                // lets get the length of the time index from the beginning of
                // the string to the '.' character that is used to align
                // all the time index entries in nice vertical line
                String subString = entry.dataSource.timeIndexDisplay.substring(0,
                        entry.dataSource.timeIndexDisplay.indexOf('.'));

                // draw the entry with horizontal position being the
                // xTimeIndexOffset minus the length of the string to the
                // '.' character
                g.drawString(entry.dataSource.timeIndexDisplay
                        + String.valueOf(Character.toChars(171)), PopUpUtils.xTimeIndexOffset
                        - m_fontMetrics.stringWidth(subString), (dimRowSize.height)
                        * (entry.displayIndex + 1) + 4);
            }
            else
            {

                // we display the message number index here (eg. 1, 2, 3, ...,
                // n)
                g.drawString(String.valueOf(entry.displayIndex), 29 - m_fontMetrics
                        .stringWidth(String.valueOf(entry.displayIndex)), (dimRowSize.height)
                        * (entry.displayIndex + 1) + 4);

                // lets get the length of the time index from the beginning of
                // the string to the '.' character that is used to align
                // all the time index entries in nice vertical line
                String subString = entry.dataSource.timeIndexDisplay.substring(0,
                        entry.dataSource.timeIndexDisplay.indexOf('.'));

                // draw the entry with horizontal position being the
                // xTimeIndexOffset minus the length of the string to the
                // '.' character
                g.drawString(entry.dataSource.timeIndexDisplay, PopUpUtils.xTimeIndexOffset
                        - m_fontMetrics.stringWidth(subString), (dimRowSize.height)
                        * (entry.displayIndex + 1) + 4);
            }

        }
    }

    public void paint(Graphics g)
    {
        int iEntries = m_model.getSize();
        Dimension dimSize = getSize();
        Dimension dimRowSize = getMinimumSize();
        int iNumKeys = m_model.getNumKeys();

        // Paint Background
        // (In case the data panel is smaller than the viewport, we
        // have already set the viewport's background color to black
        // also.)
        g.setColor(Color.black);
        g.fillRect(0, 0, dimSize.width, dimSize.height);

        if ((iEntries > 0) && (dimRowSize.width > 0) && (iNumKeys > 0))
        {
            for (int i = 0; i < iEntries; i++)
            {
                // first we paint the alternating black and dark
                // gray alternating rows
                paintBackgroundRow(g, i);
            }
        }

        // Paint all elements
        if ((iEntries > 0) && (dimRowSize.width > 0) && (iNumKeys > 0))
        {
            for (int i = 0; i < iEntries; i++)
            {
                paintEntry(g, i);
            }
        }
    }

    public Dimension getPreferredSize()
    {
        int width = DEFAULT_MAX_WIDTH_PER_KEY * (m_model.getNumKeys() + 1);
        int height = DEFAULT_MAX_HEIGHT;

        Graphics g = getGraphics();
        if (g != null)
        {
            Font f = getFont();
            if (f != null)
            {
                FontMetrics fm = g.getFontMetrics(f);
                height = fm.getHeight() + ARROW_HEIGHT;
            }
        }

        // the preferred size of the display pane must be
        // returned as 0 when all dialogs are removed, if it
        // is not then the height of ARROWN_HEIGHT vertical
        // lines will be displayed but we actually want all the
        // artifacts to be hidden, for a nice clean look
        if (m_model.getDisplaySize() == 0)
        {
            height = 0;
        }
        else
        {
            height = height * (m_model.getDisplaySize() + 1);
        }

        return new Dimension(width, height);
    }

    public Dimension getMinimumSize()
    {
        int width = DEFAULT_MAX_WIDTH_PER_KEY * (m_model.getNumKeys() + 1);
        int height = DEFAULT_MAX_HEIGHT;

        Graphics g = getGraphics();
        if (g != null)
        {
            Font f = getFont();
            if (f != null)
            {
                FontMetrics fm = g.getFontMetrics(f);
                height = fm.getHeight() + ARROW_HEIGHT;
            }
        }

        return new Dimension(width, height);
    }

    public Dimension getPreferredScrollableViewportSize()
    {
        return getPreferredSize();
    }

    public int getScrollableBlockIncrement(Rectangle visibleRect, int orientation, int direction)
    {
        // The visible area height, minus one arrow spacing.
        return visibleRect.height - getMinimumSize().height;
    }

    public boolean getScrollableTracksViewportHeight()
    {
        return false;
    }

    public boolean getScrollableTracksViewportWidth()
    {
        return true;
    }

    public int getScrollableUnitIncrement(Rectangle visibleRect, int orientation, int direction)
    {
        // The spacing between arrows.
        return getMinimumSize().height;
    }

    // ////////////////////////////////////////////////////////////////////////////
    // Implementation
    // //
    protected int PointToIndex(Point point)
    {
        Dimension size = getMinimumSize();
        int index = (point.y - (size.height / 2)) / size.height;

        if (index >= m_model.getSize())
        {
            index = -1;
        }
        else
        {
            // we want to return the index of the entry that is
            // actually being displayed and not the raw original
            // index that was assigned to it
            for (int i = 0; i < m_model.m_iNumEntries; i++)
            {
                // we loop through all the messages and find which
                // message has the display index that corresponds
                // to the current location
                if (m_model.m_entries[i].displayIndex == index)
                {
                    // we found the entry that is being displayed
                    // lets return its raw sequence index originally
                    // assigned, so that proper sip message can
                    // be displayed to the user
                    return i;
                }
            }
        }

        return index;
    }

    // when user moves the mouse over a significant area
    // this method is called
    protected void setMouseOver(int iIndex)
    {
        iOldIndex = m_iMouseOver;
        m_iMouseOver = iIndex;

        // do work only if the previous index
        // is different from the current index
        if (iOldIndex != m_iMouseOver)
        {
            Graphics g = getGraphics();
            if (g != null)
            {
                // if the old index was not -1 that means
                // we had some messages painted, we know that
                // we are dealing with a new index now so the
                // previous painted messages have to be cleared
                // by repainting them white
                if (iOldIndex != -1)
                {

                    repaintAllMatching(iOldIndex);
                }

                // if the new position is on a new index lets
                // repaint the new messages that correspond to
                // the new index
                if (m_iMouseOver != -1)
                {
                    // repaint messages and set the info panel to the
                    // currently selected message
                    repaintAllMatching(m_iMouseOver);
                    ;
                }
            }
        }
    }

    /**
     * Repaint all entries that match the branchId
     */
    protected void repaintAllMatching(int iSourceIndex)
    {
        String strCallId = null;

        if ((iSourceIndex >= 0) && (iSourceIndex < m_model.getSize()))
        {
            ChartDescriptor sourceDesc = m_model.getEntryAt(iSourceIndex);
            strCallId = sourceDesc.dataSource.getCallId();

            if ((strCallId != null) && (strCallId.length() > 0))
            {
                Graphics g = getGraphics();
                if (g != null)
                {
                    for (int i = 0; i < m_model.getSize(); i++)
                    {
                        ChartDescriptor desc = m_model.getEntryAt(i);
                        if (strCallId.equals(desc.dataSource.getCallId()))
                        {
                            paintEntry(g, i);
                        }
                    }
                }
            }
            else
            {
                Graphics g = getGraphics();
                if (g != null)
                {
                    paintEntry(g, iSourceIndex);
                }
            }
        }
    }

    // ////////////////////////////////////////////////////////////////////////////
    // Nested classes
    // //

    protected class icMouseListener implements MouseListener
    {
        public void mouseClicked(MouseEvent e)
        {

            if (e.isPopupTrigger())
            {
                // if user invokes PopupTrigger (usually a right click)
                // show the popup menu
                m_toolsPopUp.show(e.getComponent(), e.getX(), e.getY());
            }
        }

        public void mousePressed(MouseEvent e)
        {
            // on different OSes popup triggers are different, that is
            // why we much check for popup trigger on mouseClicked and also
            // on mousePressed and mouseRelease, to cover all possibilities
            if (e.isPopupTrigger())
            {
                // show popup menu
                m_toolsPopUp.show(e.getComponent(), e.getX(), e.getY());
            }
        }

        public void mouseReleased(MouseEvent e)
        {
            if (e.isPopupTrigger())
            {
                // show popup menu
                m_toolsPopUp.show(e.getComponent(), e.getX(), e.getY());
            }
        }

        public void mouseEntered(MouseEvent e)
        {
        }

        public void mouseExited(MouseEvent e)
        {
            // repaint only if the popup menu is not displayed, if it is then we
            // don't want to repaint the background screen since it paints over
            // the popup menu
            if (!m_frame.m_body.m_toolsPopUp.isShowing()
                    && !m_frame.m_bodySecond.m_toolsPopUp.isShowing()
                    && !m_frame.m_bodyTimeIndex.m_toolsPopUp.isShowing()
                    && !m_frame.m_bodyTimeIndexSecond.m_toolsPopUp.isShowing())
            {
                // handle the main scroll pane and its time index pane,
                // even if its invisible
                m_frame.m_body.setMouseOver(-1);
                m_frame.m_bodyTimeIndex.setMouseOver(-1);

                // if the bottom chart is also visible (user is working in
                // split screen mode) then also repaint messages on that
                // ChartBody
                if (m_frame.getPaneVisibility(SIPViewerFrame.bottomPaneID))
                {
                    // handle the second scroll pane and its time index
                    // pane even if its invisible
                    m_frame.m_bodySecond.setMouseOver(-1);
                    m_frame.m_bodyTimeIndexSecond.setMouseOver(-1);
                }
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
            // don't want to repaint the background screen since it paints over
            // the popup menu, this goes for the pop-up menu for any of the
            // time index panes and the main display panes
            if (!m_frame.m_body.m_toolsPopUp.isShowing()
                    && !m_frame.m_bodySecond.m_toolsPopUp.isShowing()
                    && !m_frame.m_bodyTimeIndex.m_toolsPopUp.isShowing()
                    && !m_frame.m_bodyTimeIndexSecond.m_toolsPopUp.isShowing())
            {
                int verticalIndex = PointToIndex(e.getPoint());

                // call the setMouseOver for the top chart to repaint
                // the highlighted messages, also reset/repaint the time
                // in time index column
                m_frame.m_body.setMouseOver(verticalIndex);
                m_frame.m_bodyTimeIndex.setMouseOver(verticalIndex);

                // if the bottom chart is also visible (user is working in
                // split screen mode) then also repaint messages on that
                // ChartBody and on its corresponding time index column
                if (m_frame.getPaneVisibility(SIPViewerFrame.bottomPaneID))
                {
                    m_frame.m_bodySecond.setMouseOver(verticalIndex);
                    m_frame.m_bodyTimeIndexSecond.setMouseOver(verticalIndex);
                }
            }
        }
    }

    protected class icChartModelListener implements ChartModelListener
    {
        public void keyAdded(int position)
        {
            repaint();
        }

        public void keyDeleted(int position)
        {
            repaint();
        }

        public void keyMoved(int oldPosition, int newPosition)
        {
            repaint();
        }

        public void bodyToHeaderRepaint()
        {
            revalidate();
            repaint();
        }

        public void entryAdded(int startPosition, int endPosition)
        {
            repaint();
        }

        public void entryDeleted(int startPosition, int endPosition)
        {
            repaint();
        }
    }

    // initalize the popup menu for this panel
    protected void initPopUp()
    {
        JMenuItem menuItem;

        m_toolsPopUp = new JPopupMenu();
        menuItem = new JMenuItem(PopUpUtils.Item11);
        menuItem.addActionListener(this);
        m_toolsPopUp.add(menuItem);

        menuItem = new JMenuItem(PopUpUtils.Item22);
        menuItem.addActionListener(this);
        m_toolsPopUp.add(menuItem);

        menuItem = new JMenuItem(PopUpUtils.Item33);
        menuItem.addActionListener(this);
        m_toolsPopUp.add(menuItem);

        menuItem = new JMenuItem(PopUpUtils.Item44);
        menuItem.addActionListener(this);
        m_toolsPopUp.add(menuItem);

        menuItem = new JMenuItem(PopUpUtils.Item55);
        menuItem.addActionListener(this);
        m_toolsPopUp.add(menuItem);

        m_toolsPopUp.addSeparator();

        menuItem = new JMenuItem(PopUpUtils.Item66);
        menuItem.addActionListener(this);
        m_toolsPopUp.add(menuItem);
    }

    @Override
    public void actionPerformed(ActionEvent arg0)
    {
        // pop-up menu action handler, here we set the time
        // display format and call the method that actually
        // recalculates time indexes, adjusts column width,
        // etc.
        if (arg0.getActionCommand().compareTo(PopUpUtils.Item11) == 0)
        {
            PopUpUtils.currentTimeDisplaySelection = PopUpUtils.TimeDisplayMode.DATE_AND_TIME;

            PopUpUtils.setTimeIndex(m_frame);
        }
        else if (arg0.getActionCommand().compareTo(PopUpUtils.Item22) == 0)
        {
            PopUpUtils.currentTimeDisplaySelection = PopUpUtils.TimeDisplayMode.TIME_OF_DAY_DEFAULT;

            PopUpUtils.setTimeIndex(m_frame);
        }
        else if (arg0.getActionCommand().compareTo(PopUpUtils.Item33) == 0)
        {
            PopUpUtils.currentTimeDisplaySelection = PopUpUtils.TimeDisplayMode.SINCE_PREVIOUS;

            PopUpUtils.setTimeIndex(m_frame);
        }
        else if (arg0.getActionCommand().compareTo(PopUpUtils.Item44) == 0)
        {
            PopUpUtils.currentTimeDisplaySelection = PopUpUtils.TimeDisplayMode.SINCE_BEGINNING;

            PopUpUtils.setTimeIndex(m_frame);
        }
        else if (arg0.getActionCommand().compareTo(PopUpUtils.Item55) == 0)
        {
            PopUpUtils.currentTimeDisplaySelection = PopUpUtils.TimeDisplayMode.SINCE_KEY_INDEX;

            PopUpUtils.setTimeIndex(m_frame);
        }
        else if (arg0.getActionCommand().compareTo(PopUpUtils.Item66) == 0)
        {

            // we take the message index on which the user opened
            // the pop-up menu and set IT as the key index, then
            // recalculate the time values, but only if the
            // time mode selected is of tyme since key index
            PopUpUtils.keyIndex = iOldIndex;
            
            if (PopUpUtils.currentTimeDisplaySelection == PopUpUtils.TimeDisplayMode.SINCE_KEY_INDEX)
            {
                PopUpUtils.setTimeIndex(m_frame);
            }
        }

        // re-validate and repaint the windows
        m_frame.m_bodyTimeIndex.revalidate();
        m_frame.m_bodyTimeIndex.repaint();
        m_frame.m_bodyTimeIndexSecond.revalidate();
        m_frame.m_bodyTimeIndexSecond.repaint();

        m_frame.validate();
        m_frame.repaint();
        
       
    }
}
