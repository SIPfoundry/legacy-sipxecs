/*
 * 
 * 
 * Copyright (C) 2009 Nortel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.callcontroller;

import java.util.TimerTask;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxrest.RestServer;

/**
 * One of these associated with a dialog. Stores the current call status.
 * (sent to us via a NOTIFY ).
 */
public class DialogContext {
    
   
    private static Logger logger = Logger.getLogger(DialogContext.class);
    private String key;
    private String status;
    
    public DialogContext() {
       
    }
    
    public void setKey(String key ) {
        this.key = key;
    }
    
    

    public void remove() {
        RestServer.timer.schedule(new TimerTask() {
            @Override
            public void run() {
                try {
                    SipUtils.getInstance().removeDialogContext(key);
                } catch (Exception ex) {
                    logger.error("Exception caught creating instance", ex);
                }
            }
        }, 30*1000);  
    
    }
    
    public void setStatus(String status) {
        this.status = status;
    }

    /**
     * @return the status
     */
    public String getStatus() {
        return status;
    }
    

}
