/**
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.util;

import java.util.Random;

import org.apache.commons.lang.RandomStringUtils;

/**
 * Returns a simple unique-ish ID string (hash) generated from the specified seed.
 * The string is intentionally easy to read and verbalize.
 * <p>
 * Because java.util.Random is used, a given seed string will result in the same
 * ID across any (compliant) Java implementation on any platform.
 * <p>
 * The specified seed is set to lower case before actually being used to compute
 * a java.util.Random seed value.
 *
 * @author Paul Mossman
 */
public class ShortHash {

    /**
     * A subset of characters and digits that are unlikely to be confused with each
     * other when read verbalized.
     *
     * @see getUniqueId
     */
   public static final char ID_CHARS[] = {
        'A', 'B',
        // C, D, E, & G sound like B
        'F', 'H', 'J',
        // I sounds like Y, and can be confused with 1
        // K sounds like J
        'L', 'M',
        // N sounds like M
        // O can be confused with 0
        // P sounds like B
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y',
        // Z, "zed" or "zee"?
        '1', '2', '3', '4', '5', '6', '7', '8', '9',
    };

    public static final short ID_LENGTH = 3;

    /**
      * Returns the hash.
      *
      * @param   seed_string the string to use as a seed for the hash
      * @return  the hash
      * @see     ID_CHARS
      */
    public static String get(String seed_string) {

        if (null == seed_string) {
            seed_string = "";
        }
        seed_string = seed_string.toLowerCase();
        Long seed_num = new Long(1);
        for (int i = 0; i < seed_string.length(); i++) {
            seed_num += (seed_num * seed_string.charAt(i)) % (Long.MAX_VALUE - 1 / seed_string.length());
        }

        // A given seed string will generate the same ID across an Java implementation,
        // and on any platform.
        return RandomStringUtils.random(ID_LENGTH, 0, ID_CHARS.length - 1, false, false, ID_CHARS, new Random(seed_num));
    }
}
