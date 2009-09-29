/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig;

/**
 * To be run from ANT scripts when setting up database
 */
public class ClearDb {

    public static void main(String[] args) {
        try {
            TestHelper.cleanInsert("ClearDb.xml");
            // system exit is nec. or ant will not return
            System.exit(0);
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}
