package org.sipfoundry.sipxrelay;


/**
 * A work item to be performed by the data shuffler thread (so we dont need any locking).
 * @author mranga
 *
 */
public abstract class WorkItem implements Comparable<WorkItem> {
    boolean error;
    String reason;
    int errorCode;
    long creationTime;
 
    protected WorkItem() {
    	this.creationTime = System.currentTimeMillis();
       
    }
    
    @Override
    public int compareTo(WorkItem workItem) {
    	if ( workItem.creationTime < this.creationTime ) {
    		return -1;
    	} else if ( creationTime >= this.creationTime ) {
    		return 1;
    	} else {
    		return 0;
    	}
    }
    
    public abstract void doWork();
    
    public void error(int errorCode, String reason) {
        this.error = true;
        this.reason = reason;
        this.errorCode = errorCode;
    }
}
