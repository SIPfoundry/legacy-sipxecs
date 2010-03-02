package com.pingtel.sipviewer;

/*
 * @(#) GUIUtils
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

import java.awt.*;
import java.util.*;

public class GUIUtils
{
    /** Align the component to the east along the horizonal axis. */
    public static final int ALIGN_EAST = 0x0010;
    /** Align the component to the west along the horizonal axis. */
    public static final int ALIGN_WEST = 0x0001;
    /** Align the component to the north along the vertical axis. */
    public static final int ALIGN_NORTH = 0x1000;
    /** Align the component to the south along the vertical axis. */
    public static final int ALIGN_SOUTH = 0x0100;
    /**
     * Align the component to the center of both the horizonal and vertical
     * axes. This is the default if a horizontal or vertical alignment is not
     * specified.
     */
    public static final int ALIGN_CENTER = 0;

    /**
     * Determine of the passed component is a within the passed container. This
     * is something that was left off the container implementation IMHO.
     */
    public static boolean componentInContainer(Component comp, Container cont)
    {
        Component comps[] = cont.getComponents();
        boolean bFound = false;

        for (int i = 0; i < comps.length; i++)
        {
            if (comps[i] == comp)
            {
                bFound = true;
                break;
            }
        }
        return bFound;
    }

    /**
     * Calculate the desired X Offset given the passed graphics context, string,
     * font, and alignment constraint.
     */
    public static int calcXOffset(String strText, Graphics g, Rectangle rectBounds, int iAlignment)
    {
        FontMetrics fm = g.getFontMetrics();
        int xOffset = 0;
        int iWidth;

        if ((fm != null) && (strText != null))
        {
            iWidth = fm.stringWidth(strText);

            // Align Flush Right
            if ((iAlignment & ALIGN_EAST) == ALIGN_EAST)
            {
                xOffset = rectBounds.x + Math.max(rectBounds.width - iWidth, 0);
            }
            // Align Flush Left
            else if ((iAlignment & ALIGN_WEST) == ALIGN_WEST)
            {
                xOffset = rectBounds.x;
            }
            // Center
            else
            {
                xOffset = rectBounds.x + Math.max((rectBounds.width - iWidth) / 2, 0);
            }
        }

        return xOffset;
    }

    /**
     * Calculate the desired Y Offset given the passed graphics context, string,
     * font, and alignment constraint.
     */
    public static int calcYOffset(String strText, Graphics g, Rectangle rectBounds, int iAlignment)
    {
        FontMetrics fm = g.getFontMetrics();
        int yOffset = 0;
        int iHeight = 0;

        if ((fm != null) && (strText != null))
        {
            iHeight = fm.getAscent();

            // Align on bottom
            if ((iAlignment & ALIGN_SOUTH) == ALIGN_SOUTH)
            {
                yOffset = rectBounds.y
                        + Math.max((rectBounds.height - (iHeight + fm.getDescent())), 0);
            }
            // Align on Top
            else if ((iAlignment & ALIGN_NORTH) == ALIGN_NORTH)
            {
                yOffset = rectBounds.y;
            }
            // Align Center
            else
            {
                yOffset = rectBounds.y + Math.max((rectBounds.height - iHeight) / 2, 0);
            }
        }
        return yOffset + iHeight;
    }

    /**
     * Calculate the desired X offet given the specified image, bounding
     * rectangle, alignment.
     */
    public static int calcXImageOffset(Image image, Rectangle rectBounds, int iAlignment)
    {
        int xOffset = 0;
        int iWidth;

        if (image != null)
        {
            iWidth = image.getWidth(null);

            // Align Flush Right
            if ((iAlignment & ALIGN_EAST) == ALIGN_EAST)
            {
                xOffset = rectBounds.x + Math.max(rectBounds.width - iWidth, 0);
            }
            // Align Flush Left
            else if ((iAlignment & ALIGN_WEST) == ALIGN_WEST)
            {
                xOffset = rectBounds.x;
            }
            // Center
            else
            {
                xOffset = rectBounds.x + Math.max((rectBounds.width - iWidth) / 2, 0);
            }
        }

        return xOffset;
    }

    /**
     * Calculate the desired Y offet given the specified image, bounding
     * rectangle, alignment.
     */
    public static int calcYImageOffset(Image image, Rectangle rectBounds, int iAlignment)
    {
        int yOffset = 0;
        int iHeight = 0;

        if (image != null)
        {
            iHeight = image.getHeight(null);

            // Align on bottom
            if ((iAlignment & ALIGN_SOUTH) == ALIGN_SOUTH)
            {
                yOffset = rectBounds.y + Math.max((rectBounds.height - iHeight) - 2, 0);
            }
            // Align on Top
            else if ((iAlignment & ALIGN_NORTH) == ALIGN_NORTH)
            {
                yOffset = rectBounds.y;
            }
            // Align Center
            else
            {
                yOffset = rectBounds.y + Math.max((rectBounds.height - iHeight) / 2, 0);
            }
        }
        return yOffset;
    }

    public static final int LINE_SOLID = 0;
    public static final int LINE_DASHED = 1;
    public static final int LINE_DOTTED = 2;
    public static final int LINE_THICK = 3;

    private static final int LINE_DASHED_SOLID_RUN = 12;
    private static final int LINE_DASHED_EMPTY_RUN = 4;

    private static final int LINE_DOTTED_SOLID_RUN = 2;
    private static final int LINE_DOTTED_EMPTY_RUN = 2;

    public static void drawArrow(Graphics g, Rectangle rect, boolean bEast, Color color,
            Color backgroundColor, int style)
    {
        Color colorOld = null;

        if (color != null)
        {
            colorOld = g.getColor();
            g.setColor(color);
        }

        int diff = rect.height / 2;
        int yOffset = rect.y + diff;

        // if the background color for this arrow is not black lets paint it
        if (backgroundColor != Color.BLACK)
        {
            // Temporarily store the current color
            Color colorTmp = g.getColor();

            // set the background color
            g.setColor(backgroundColor);

            // use magic numbers to adjust the dimensions of the rectangle,
            // these number were derived through experimentation and were
            // chosen to perfectly cover the arrow

            if (rect.width < 0)
            {
                g.fillRect(rect.x + rect.width - diff, rect.y, -rect.width + diff, rect.height + 2);
            }
            else
            {
                g.fillRect(rect.x - 3, rect.y, rect.width + 6, rect.height + 2);
            }

            g.setColor(colorTmp);
        }

        switch (style)
        {
        case GUIUtils.LINE_DASHED:
            for (int i = 0; i < rect.width; i++)
            {
                int x1 = rect.x + i;
                int x2 = x1 + LINE_DASHED_SOLID_RUN;
                if (x2 > rect.x + rect.width)
                {
                    x2 = rect.x + rect.width;
                }

                g.drawLine(x1, yOffset, x2, yOffset);

                i = i + LINE_DASHED_SOLID_RUN + LINE_DASHED_EMPTY_RUN;
            }
            break;

        case GUIUtils.LINE_DOTTED:
            for (int i = 0; i < rect.width; i++)
            {
                int x1 = rect.x + i;
                int x2 = x1 + LINE_DOTTED_SOLID_RUN;
                if (x2 > rect.x + rect.width)
                {
                    x2 = rect.x + rect.width;
                }

                g.drawLine(x1, yOffset, x2, yOffset);

                i = i + LINE_DOTTED_SOLID_RUN + LINE_DOTTED_EMPTY_RUN;
            }
            break;

        case GUIUtils.LINE_THICK:
            // thick line is drawn just like a solid line but 3 times with
            // +/- 1 vertical index differential
            g.drawLine(rect.x, yOffset - 1, rect.x + rect.width, yOffset - 1);
            g.drawLine(rect.x, yOffset, rect.x + rect.width, yOffset);
            g.drawLine(rect.x, yOffset + 1, rect.x + rect.width, yOffset + 1);
            break;

        default:
            g.drawLine(rect.x, yOffset, rect.x + rect.width, yOffset);
            break;
        }

        if (bEast)
        {
            // draw the arrow end for messages that are internal to a given
            // dialog participant
            g.drawLine(rect.x + rect.width, yOffset, rect.x + rect.width - diff, rect.y + diff
                    + diff);
            g.drawLine(rect.x + rect.width, yOffset, rect.x + rect.width - diff, rect.y);
        }
        else
        {
            // draw the end of the arrow, the pointy end
            g.drawLine(rect.x, yOffset, rect.x + diff, rect.y + diff + diff);
            g.drawLine(rect.x, yOffset, rect.x + diff, rect.y);
        }

        if (colorOld != null)
        {
            g.setColor(colorOld);
        }
    }

    /**
     * Draws a 3d frame around the designated rectangle.
     * 
     * @param g
     *            The graphics context that will be drawn upon.
     * @param rect
     *            The rectangle that marks the bounds of the 3d frame
     * @param colorBackground
     *            The background color that will be painted within the frame.
     */
    public static void draw3DFrame(Graphics g, Rectangle rect, Color colorBackground,
            Color colorDark, Color colorLight)
    {
        Color colorOld = g.getColor();

        // Fill Background Area
        g.setColor(colorBackground);
        g.fillRect(rect.x, rect.y, rect.width, rect.height);

        // Draw borders
        g.setColor(colorDark);

        // __
        // x__|
        //
        g.drawLine(rect.x, rect.y, rect.x, rect.y + (rect.height - 1));

        // xx
        // |__|
        //
        g.drawLine(rect.x, rect.y, rect.x + (rect.width - 1), rect.y);

        g.setColor(colorLight);

        // __
        // | |
        // xx
        g.drawLine(rect.x + 1, rect.y + (rect.height - 1), rect.x + (rect.width - 1), rect.y
                + (rect.height - 1));

        // __
        // |__x
        //
        g.drawLine(rect.x + (rect.width - 1), rect.y + 1, rect.x + (rect.width - 1), rect.y
                + (rect.height - 1));

        g.setColor(colorOld);
    }

    /**
     * Draws a 3d hilite frame around the designated rectangle.
     * 
     * @param g
     *            The graphics context that will be drawn upon.
     * @param rect
     *            The rectangle that marks the bounds of the 3d frame
     * @param colorBackground
     *            The background color that will be painted within the frame.
     */
    public static void draw3dHiliteFrame(Graphics g, Rectangle rect, Color colorDark,
            Color colorLight)
    {
        Color colorOld = g.getColor();

        // Draw borders
        g.setColor(colorDark);

        /*
         * First draw a dark rectangle
         */
        g.drawRect(rect.x, rect.y, rect.width - 2, rect.height - 2);

        /*
         * Fill in lite areas
         */
        g.setColor(colorLight);

        // __
        // x__|
        //
        g.drawLine(rect.x + 1, rect.y + 1, rect.x + 1, rect.y + (rect.height - 3));

        // xx
        // |__|
        //
        g.drawLine(rect.x + 1, rect.y + 1, rect.x + (rect.width - 3), rect.y + 1);

        // __
        // | |
        // xx
        g.drawLine(rect.x + 1, rect.y + (rect.height - 1), rect.x + (rect.width - 1), rect.y
                + (rect.height - 1));

        // __
        // |__x
        //
        g.drawLine(rect.x + (rect.width - 1), rect.y, rect.x + (rect.width - 1), rect.y
                + (rect.height - 1));

        g.setColor(colorOld);
    }

    // this method is used to draw the background colors for the alternating
    // rows
    public static void paintBackgroundRow(Graphics g, Rectangle rectangleArea)
    {
        // store current color whatever it is
        Color tmpColor = g.getColor();

        // set the background color (determined by experimentation)
        g.setColor(new Color(20, 18, 20));

        // fill the background with the color
        g.fillRect(rectangleArea.x, rectangleArea.y, rectangleArea.width, rectangleArea.height);

        // reset to original color
        g.setColor(tmpColor);
    }

    // this method is used to draw the background with a custom
    // specified color
    public static void paintBackgroundRowCustomColor(Graphics g, Rectangle rectangleArea,
            Color customColor)
    {
        // store current color whatever it is
        Color tmpColor = g.getColor();

        // set color to the specified custom color
        g.setColor(customColor);

        // fill the background with the color
        g.fillRect(rectangleArea.x, rectangleArea.y, rectangleArea.width, rectangleArea.height);

        // reset to original color
        g.setColor(tmpColor);
    }

}
