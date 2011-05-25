/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package com.pingtel.sipviewer;

import java.awt.Color;

// contains information about each "message"
// the source columns to draw the arrow from to
// the destination arrow, the message text
// and the background color for the label and
// for the arrow, also it contains the display index
public class ChartDescriptor
{
    public int sourceColumn;
    public String source;
    public int targetColumn;
    public String target;
    public String label;

    // background color for the arrow
    public Color backgroundColor;

    // when a file is loaded all the messages are assigned a
    // display index, which is simply a sequence of numbers from
    // 0 to numOfMessages, this is used by sipviewer to know where
    // each message should be vertically pained on the screen
    public int displayIndex;

    public SipBranchData dataSource;
}
