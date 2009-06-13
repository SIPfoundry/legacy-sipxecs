package org.sipfoundry.sipxbridge.symmitron;

import java.util.concurrent.Semaphore;

/**
 * A work item to be performed by the data shuffler thread (so we dont need any locking).
 * @author mranga
 *
 */
public abstract class WorkItem {
 
    protected WorkItem() {
       
    }
    
    public abstract void doWork();
}
