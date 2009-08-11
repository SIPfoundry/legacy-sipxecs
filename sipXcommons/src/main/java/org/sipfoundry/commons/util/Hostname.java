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
 * the local machine running the JVM.
 *
 * @author Mardy Marshall
 */
public class Hostname {
    static {
        try {
            LibraryLoader.loadLibrary("hostname");
        } catch (UnsatisfiedLinkError e) {
            e.printStackTrace();
        }
    }

    public native static String get();
}
