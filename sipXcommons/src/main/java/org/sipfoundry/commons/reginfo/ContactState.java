/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.reginfo;

/**
 * Enumerations for contact state attribute.
 * <p>
 *
 * @author Mardy Marshall
 */
public enum ContactState {
	/**
	 * Contact is active.
	 */
	ACTIVE("active"),

	/**
	 * Contact has been terminated.
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
	 * Default constructor.
	 *
	 * @param stringValue
	 *            String representation of enumeration.
	 */
	ContactState(String stringValue) {
		this.stringValue = stringValue;
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
	public static ContactState toEnum(String stringValue) {
		if (stringValue.compareToIgnoreCase("active") == 0) {
			return ACTIVE;
		} else if (stringValue.compareToIgnoreCase("terminated") == 0) {
			return TERMINATED;
		} else {
			return INVALID;
		}
	}
}
