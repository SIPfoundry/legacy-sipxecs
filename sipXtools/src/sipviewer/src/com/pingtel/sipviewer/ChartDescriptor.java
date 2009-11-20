package com.pingtel.sipviewer;

import java.awt.Color;

// contains information about each "message"
// the source columns to draw the arrow from to
// the destination arrow, the message text
// and the background color for the label and
// for the arrow
public class ChartDescriptor
{
    public int    sourceColumn ;
    public String source ;
    public int    targetColumn ;
    public String target ;
    public String label ;
    public Color  backgroundColor ;

    public SipBranchData dataSource ;
}
