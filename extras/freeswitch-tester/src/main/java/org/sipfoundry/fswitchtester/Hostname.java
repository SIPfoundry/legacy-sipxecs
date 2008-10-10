/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.fswitchtester;

import java.io.BufferedReader;
import java.io.InputStreamReader;

/**
 * A Hack to get the Host Name of the machine running the JVM.
 * <p>
 * Tries various and sundry ways to get it, either from the command line
 * property "HOSTNAME", the environment variable "HOSTNAME", by running the
 * POSIX "uname -n" command, and finally trying a reverse DNS lookup from what
 * java.net.InetAddress.getLocalHost() returns.
 * 
 * @author Woof!
 * 
 */
public class Hostname {
    static String hostName = null;

    public static String get() {
        if (hostName == null) {
            // First try a command line property
            hostName = System.getProperty("HOSTNAME");
        }

        if (hostName == null) {
            // Next try the environment
            hostName = System.getenv("HOSTNAME");
        }

        if (hostName == null) {
            // Next, assume POSIX and run uname -n
            try {
                Process uname = Runtime.getRuntime().exec("uname -n");
                BufferedReader input = new BufferedReader(
                        new InputStreamReader(uname.getInputStream()));
                hostName = input.readLine();
                int ret = uname.waitFor();
                if (ret != 0) {
                    hostName = null;
                }
            } catch (Exception e) {
            }
        }

        if (hostName == null) {
            // Last, do the reverse DNS lookup
            try {
                hostName = java.net.InetAddress.getLocalHost().getHostName();
            } catch (Exception e) {
            }
        }

        return hostName;
    }
}
