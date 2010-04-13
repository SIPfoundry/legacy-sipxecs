/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.reginfo;

/**
 * Enumerations for reginfo state attribute.
 * <p>
 *
 * @author Mardy Marshall
 */
public enum ReginfoState {
	/**
	 * Reginfo contains full list of registrations.
	 */
	FULL("full"),

	/**
	 * Reginfo contains partial list of registrations.
	 */
	PARTIAL("partial"),

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
	ReginfoState(String stringValue) {
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
	public static ReginfoState toEnum(String stringValue) {
		if (stringValue.compareToIgnoreCase("full") == 0) {
			return FULL;
		} else if (stringValue.compareToIgnoreCase("partial") == 0) {
			return PARTIAL;
		} else {
			return INVALID;
		}
	}
}
