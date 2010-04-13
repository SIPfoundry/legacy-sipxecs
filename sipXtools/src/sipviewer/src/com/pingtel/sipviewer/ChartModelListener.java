/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package com.pingtel.sipviewer;

public interface ChartModelListener
{
    public void keyAdded(int position) ;
    public void keyDeleted(int position) ;
    public void keyMoved(int oldPosition, int newPosition) ;
    
    public void bodyToHeaderRepaint();

    public void entryAdded(int startPosition, int endPosition) ;
    public void entryDeleted(int startPosition, int endPosition) ;
}
