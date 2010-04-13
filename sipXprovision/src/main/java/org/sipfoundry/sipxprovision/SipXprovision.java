/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxprovision;

import org.sipfoundry.sipxprovision.auto.Servlet;

// TODO javadoc
public class SipXprovision {

    /**
     * Main entry point for sipXprovision
     * @param args
     */
    public static void main(String[] args) {
        try {

            System.out.println("Hello world!");

            Servlet.start();

            // TODO: addShutdownHook

            for (;;){
                try { Thread.sleep(1000); } catch (InterruptedException e) { }
            }
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        } catch (Throwable t) {
            t.printStackTrace();
            System.exit(1);
        }
    }
}
