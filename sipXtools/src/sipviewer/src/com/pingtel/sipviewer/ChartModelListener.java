package com.pingtel.sipviewer;

public interface ChartModelListener
{
    public void keyAdded(int position) ;
    public void keyDeleted(int position) ;
    public void keyMoved(int oldPosition, int newPosition) ;

    public void entryAdded(int startPosition, int endPosition) ;
    public void entryDeleted(int startPosition, int endPosition) ;
}
