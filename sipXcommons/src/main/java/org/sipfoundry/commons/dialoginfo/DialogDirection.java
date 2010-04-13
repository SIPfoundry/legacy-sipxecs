/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

/**
 * Enumerations for dialog direction attribute.
 * <p>
 *
 * @author Mardy Marshall
 */
public enum DialogDirection {
	/**
	 * Dialog is from session initiator.
	 */
	INITIATOR("initiator"),

	/**
	 * Dialog is from session recipient.
	 */
	RECIPIENT("recipient"),

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
	DialogDirection(String stringValue) {
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
	public static DialogDirection toEnum(String stringValue) {
		if (stringValue.compareToIgnoreCase("initiator") == 0) {
			return INITIATOR;
		} else if (stringValue.compareToIgnoreCase("recipient") == 0) {
			return RECIPIENT;
		} else {
			return INVALID;
		}
	}
}
