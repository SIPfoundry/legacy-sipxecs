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

import junit.framework.TestCase;

/**
 * Explicitly excersizes the spring configuration
 */
public class SpringConfigurationTestDb extends TestCase {
    public void testConfiguration() {
        TestHelper.getApplicationContext();
    }
}
