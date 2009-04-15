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
 * of the running JVM.
 * 
 * @author Mardy Marshall
 */
public class ProcessID {
    static {
        try {
            LibraryLoader.loadLibrary("processid");
        } catch (UnsatisfiedLinkError e) {
            e.printStackTrace();
        }
    }

    public native static int get();
}

