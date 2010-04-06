/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrelay;


/**
 * A work item to be performed by the data shuffler thread (so we dont need any locking).
 * @author mranga
 *
 */
public abstract class WorkItem  {
    boolean error;
    String reason;
    int errorCode;
    long creationTime;
 
    protected WorkItem() {
    	this.creationTime = System.currentTimeMillis();
       
    }
    
    
    public abstract void doWork();
    
    public void error(int errorCode, String reason) {
        this.error = true;
        this.reason = reason;
        this.errorCode = errorCode;
    }
}
