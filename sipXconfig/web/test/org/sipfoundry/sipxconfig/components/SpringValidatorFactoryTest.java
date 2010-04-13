/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

public class SpringValidatorFactoryTest extends TestCase {

    /*
     * Test method for 'org.sipfoundry.sipxconfig.components.SpringValidatorFactory.splitSpecification(String)'
     */
    public void testSplitSpecification() {
        SpringValidatorFactory factory = new SpringValidatorFactory();
        assertArrayEquals(new String[] {""}, factory.splitSpecification(""));
        assertArrayEquals(new String[] {"a", "b"}, factory.splitSpecification("a, b"));
        assertArrayEquals(new String[] {"a", "b=x", "c=y"}, factory.splitSpecification("a, b=x, c=y"));
    }

    void assertArrayEquals(String[] expected, String[] actual) {
        assertEquals(expected.length, actual.length);
        for (int i = 0; i < expected.length; i++) {
            assertEquals(expected[i], actual[i]);
        }
    }
}
