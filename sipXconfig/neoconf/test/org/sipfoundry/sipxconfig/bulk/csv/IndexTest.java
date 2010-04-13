/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import junit.framework.TestCase;

public class IndexTest extends TestCase {
    public void testLabels() {
        // make sure that people add labels when adding new columns
        assertEquals(Index.values().length, Index.labels().length);
    }

}
