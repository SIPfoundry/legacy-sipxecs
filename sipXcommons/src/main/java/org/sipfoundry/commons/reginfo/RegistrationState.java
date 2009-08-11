/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.reginfo;

import java.util.concurrent.ConcurrentHashMap;

/**
 * Enumerations for registration state attribute.
 * <p>
 *
 * @author Mardy Marshall
 */
public enum RegistrationState {
	/**
	 * Registration is in initialization state.
	 */
	INIT("init"),

	/**
	 * Registration is active.
	 */
	ACTIVE("active"),

	/**
	 * Registration is terminated.
	 */
	TERMINATED("terminated"),

	/**
	 * Indicates an invalid value.
	 */
	INVALID("invalid");

	/**
	 * Storage for string representation of enumeration.
	 */
	private String stringValue;

	/**
	 * Hash map used for mapping enums to string values.
	 */
	private static ConcurrentHashMap<String, RegistrationState> stringMap;

	/**
	 * Default constructor.
	 *
	 * @param stringValue
	 *            String representation of enumeration.
	 */
	RegistrationState(String stringValue) {
		this.stringValue = stringValue;

		// Maintain a map between enum and string value.
		mapStringValue(stringValue);
	}

	/**
	 * Retrieve the string representation of this enumeration.
	 *
	 * @return String representation of this enumeration.
	 */
	public String toString() {
		return stringValue;
	}

	/**
	 * Map the given string, ignoring case, to a corresponding ENUM value.
	 *
	 * @param stringValue to convert.
	 * @return Corresponding ENUM value.  If no match, returns INVALID.
	 */
	public static RegistrationState toEnum(String stringValue) {
		RegistrationState value = RegistrationState.stringMap.get(stringValue.toLowerCase());
		if (value == null) {
			return INVALID;
		} else {
			return value;
		}
	}

    /**
     * Maintain a map of ENUM values and strings, ignoring case.
     *
     * @param stringValue to map.
     */
    private void mapStringValue(String stringValue) {
        if (RegistrationState.stringMap == null) {
            RegistrationState.stringMap = new ConcurrentHashMap<String, RegistrationState>();
        }
        RegistrationState.stringMap.put(stringValue.toLowerCase(), this);
    }

}
