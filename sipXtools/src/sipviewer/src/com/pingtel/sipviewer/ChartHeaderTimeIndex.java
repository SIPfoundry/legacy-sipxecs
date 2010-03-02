package com.pingtel.sipviewer;

import java.awt.Color;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;

import com.pingtel.sipviewer.PopUpUtils.TimeDisplayMode;

public class ChartHeaderTimeIndex extends Component
{
    // ////////////////////////////////////////////////////////////////////////////
    // Constants
    // //
    protected static final int DEFAULT_MAX_WIDTH = 200;
    protected static final int DEFAULT_MAX_HEIGHT = 100;

    // ////////////////////////////////////////////////////////////////////////////
    // Attributes
    // //
    protected SIPViewerFrame m_frame;

    // ////////////////////////////////////////////////////////////////////////////
    // Constructors
    // //
    public ChartHeaderTimeIndex(SIPViewerFrame frame) {
        m_frame = frame;

    }

    // ////////////////////////////////////////////////////////////////////////////
    // Public Methods
    // //
    public void paint(Graphics g)
    {
        Dimension dimSize = getSize();

        // get the dimension of the window and set the background to black
        if (dimSize.width > 0)
        {
            g.setColor(Color.black);
            g.fillRect(0, 0, dimSize.width, dimSize.height);
        }

        g.setColor(Color.yellow);

        // as long as we have some elements present display time index
        // column heading
        if (m_frame.m_model.getNumKeys() != 0)
        {

            // if the time display format is date and time, time of day
            // or any other change the time index column header
            // appropriately, in case of first two also display the
            // time zone that is being used by showing GMT-x
            if (PopUpUtils.currentTimeDisplaySelection == TimeDisplayMode.DATE_AND_TIME)
            {
                // we convert gmt offset from milliseconds to seconds to minutes to hours
                g.drawString("   No.             Time (GMT" + m_frame.localTimeZone.getRawOffset()
                        / 1000 / 60 / 60 + ")", 6, 19);
            }
            else if (PopUpUtils.currentTimeDisplaySelection == TimeDisplayMode.TIME_OF_DAY_DEFAULT)
            {
                g.drawString("  No.   Time (GMT" + m_frame.localTimeZone.getRawOffset() / 1000 / 60
                        / 60 + ")", 6, 19);
            }
            else
            {
                g.drawString("   No.        Time", 6, 19);
            }
        }
    }

    public Dimension getPreferredSize()
    {
        return getMinimumSize();
    }

    public Dimension getMinimumSize()
    {
        int width = DEFAULT_MAX_WIDTH;
        int height = DEFAULT_MAX_HEIGHT;

        Graphics g = getGraphics();
        if (g != null)
        {
            Font f = getFont();
            if (f != null)
            {
                FontMetrics fm = g.getFontMetrics(f);
                height = fm.getHeight() * 2;
            }
        }
        return new Dimension(width, height);
    }
}
