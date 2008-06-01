/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxconfig.sip;

import java.io.BufferedReader;
import java.io.InputStreamReader;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * A Hack to get the Host Name of the machine running the JVM.
 * <p>
 * Tries various and sundry ways to get it, either from the command line
 * property "HOSTNAME", the environment variable "HOSTNAME", by running the
 * POSIX "uname -n" command, and finally trying a reverse DNS lookup from what
 * java.net.InetAddress.getLocalHost() returns.
 * 
 * TODO - move this to sipx commons.
 * 
 * @author Woof!
 * 
 */
public final class Hostname {
    private static Log s_logger = LogFactory.getLog(Hostname.class);
    
    private static String s_hostName;
    
    private static String s_errorMessage = "Ignoring error";
    
    private static final String HOSTNAME = "HOSTNAME";
    
    private Hostname() {
        
    }
    
    public static String get() {
        if (s_hostName == null) {
            // First try a command line property
            s_hostName = System.getProperty(HOSTNAME);
        }

        if (s_hostName == null) {
            // Next try the environment
            s_hostName = System.getenv(HOSTNAME);
        }

        if (s_hostName == null) {
            // Next, assume POSIX and run uname -n
            try {
                Process uname = Runtime.getRuntime().exec("uname -n");
                BufferedReader input = new BufferedReader(
                        new InputStreamReader(uname.getInputStream()));
                s_hostName = input.readLine();
                int ret = uname.waitFor();
                if (ret != 0) {
                    s_hostName = null;
                }
            } catch (Exception e) {
               /* Ignore */
                s_logger.error(s_errorMessage, e);
            }
        }

        if (s_hostName == null) {
            // Last, do the reverse DNS lookup
            try {
                s_hostName = java.net.InetAddress.getLocalHost().getHostName();
            } catch (Exception e) {
                /* Ignore */
                s_logger.error(s_errorMessage, e);
            }
        }

        return s_hostName;
    }
}
