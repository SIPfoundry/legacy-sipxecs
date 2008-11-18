/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.sipxconfig.admin;

import java.io.IOException;
import java.io.Serializable;

/**
 * Runs a package update completely synchronously, while the user is presented with the "waiting page".
 */
public class SynchronousPackageUpdate implements Serializable, WaitingListener {

    /** The path to the sipxpackage binary. */
    private String m_updateBinaryPath;
    
    public SynchronousPackageUpdate(String updateBinaryPath) {
        m_updateBinaryPath = updateBinaryPath;
    }
    
    public void afterResponseSent() {
        ProcessBuilder updateProcessBuilder = new ProcessBuilder(m_updateBinaryPath, "update");       
        updateProcessBuilder.redirectErrorStream(true);
        
        try {
            Process updateProcess = updateProcessBuilder.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
