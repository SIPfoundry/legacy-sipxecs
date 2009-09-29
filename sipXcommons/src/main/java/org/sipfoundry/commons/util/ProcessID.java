/**
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.util;

/**
 * Determine the process ID of the running JVM.
 * <p>
 * Using a native method, determine and return the process ID
 * of the running JVM.  If native support is not available then
 * return an -1 indicating that PID is unavailable.
 *
 * @author Mardy Marshall
 */
public class ProcessID {
    private static boolean nativeLoaded = false;
    
    static {
        try {
            LibraryLoader.loadLibrary("processid");
            nativeLoaded = true;
        } catch (UnsatisfiedLinkError e) {
            nativeLoaded = false;
            System.err.println("Process ID support not available: " + e.getMessage());
        }
    }

    private native static int getPid();
    
	public static int get() {
	    if (nativeLoaded) {
	        return getPid();
	    } else {
	        return -1;
	    }
	}
}
