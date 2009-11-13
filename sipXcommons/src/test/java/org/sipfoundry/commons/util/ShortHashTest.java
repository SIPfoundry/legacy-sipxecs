/**
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.util;

import java.util.Arrays;
import java.util.List;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

/**
 * Basic test of the ShortHash class.
 * <p>
 *
 * @author Paul Mossman
 */
public class ShortHashTest extends TestCase {

    public void testIt() {

        String seed1 = "lower case seed";

        // The hash is a constant length.
        assertEquals(ShortHash.ID_LENGTH, ShortHash.get(seed1).length());

        // The same seed must result in the same hash.
        assertEquals(ShortHash.get(seed1), ShortHash.get(seed1));

        // The seed is case insensitive.
        assertEquals(ShortHash.get(seed1), ShortHash.get(seed1.toUpperCase()));

        // No need to choke.
        assertEquals(ShortHash.get(null).length(), ShortHash.ID_LENGTH);
        assertEquals(ShortHash.get("").length(), ShortHash.ID_LENGTH);

        // The hash characters must contain only ID_CHARS.
        checkCharacterSet(null);
        checkCharacterSet("");
        checkCharacterSet(seed1);
        checkCharacterSet("another seed 123");
        checkCharacterSet("yet another seed 1234567890");
        checkCharacterSet("1 more for fun");
    }

    protected void checkCharacterSet(String seed) {

        String hash = ShortHash.get(seed);
        for (char c : ShortHash.ID_CHARS) {
            hash = hash.replace(c, '*');
        }

        String expected = "";
        for (int x=0; x < ShortHash.ID_LENGTH; x++) {
            expected += "*";
        }

        assertEquals(expected, hash);
    }

}
