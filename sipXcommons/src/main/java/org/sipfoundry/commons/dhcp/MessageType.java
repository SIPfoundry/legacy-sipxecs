/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dhcp;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public enum MessageType {
    BOOTREQUEST(1),
    BOOTREPLY(2),
    INVALID(-1);

    /**
     * Storage for integer representation of enumeration.
     */
    private int integerValue;

    /**
     * Default constructor.
     *
     * @param integerValue
     *            Integer representation of enumeration.
     */
    MessageType(int integerValue) {
        this.integerValue = integerValue;
    }

    /**
     * Retrieve the integer representation of this enumeration.
     *
     * @return Integer representation of this enumeration.
     */
    public int toInt() {
        return integerValue;
    }

    /**
     * Map the given integer to a corresponding ENUM value.
     *
     * @param integerValue
     *            to convert.
     * @return Corresponding ENUM value. If no match, returns INVALID.
     */
    public static MessageType toEnum(int integerValue) {
        switch (integerValue) {
            case 1:
                return BOOTREQUEST;
            case 2:
                return BOOTREPLY;
            default:
                return INVALID;
        }
    }

}
