/**
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.util;

/**
 * Determine the host name of the machine running the JVM.
 * <p>
 * Using a native method, determine and return the host name of
 * the local machine running the JVM.  If native support is not
 * available, then just return "local" as host name.
 *
 * @author Mardy Marshall
 */
public class Hostname {
    private static boolean nativeLoaded = false;
    
    static {
        try {
            LibraryLoader.loadLibrary("hostname");
            nativeLoaded = true;
        } catch (UnsatisfiedLinkError e) {
            nativeLoaded = false;
            System.err.println("Hostname support not available: " + e.getMessage());
        }
    }

    private native static String getHostname();
	
	public static String get() {
	    if (nativeLoaded) {
	        return getHostname();
	    } else {
	        return "local";
	    }
	}
}
