package org.sipfoundry.sipxrelay;


/**
 * A work item to be performed by the data shuffler thread (so we dont need any locking).
 * @author mranga
 *
 */
public abstract class WorkItem {
    boolean error;
    String reason;
    int errorCode;
 
    protected WorkItem() {
       
    }
    
    public abstract void doWork();
    
    public void error(int errorCode, String reason) {
        this.error = true;
        this.reason = reason;
        this.errorCode = errorCode;
    }
}
