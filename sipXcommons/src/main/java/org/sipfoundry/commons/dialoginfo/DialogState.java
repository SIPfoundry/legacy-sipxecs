/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

import java.util.concurrent.ConcurrentHashMap;

/**
 * Enumerations for dialog state.
 * <p>
 *
 * @author Mardy Marshall
 */
public enum DialogState {
	/**
	 * The dialog state is "trying".
	 */
	TRYING("trying"),

	/**
	 * The dialog state is "proceeding".
	 */
	PROCEEDING("proceeding"),

	/**
	 * The dialog state is "early".
	 */
	EARLY("early"),

	/**
	 * The dialog state is "confirmed".
	 */
	CONFIRMED("confirmed"),

	/**
	 * The dialog state is "terminated".
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
	private static ConcurrentHashMap<String, DialogState> stringMap;

	/**
	 * Default constructor.
	 *
	 * @param stringValue
	 *            String representation of enumeration.
	 */
	DialogState(String stringValue) {
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
	public static DialogState toEnum(String stringValue) {
		DialogState value = DialogState.stringMap.get(stringValue.toLowerCase());
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
        if (DialogState.stringMap == null) {
            DialogState.stringMap = new ConcurrentHashMap<String, DialogState>();
        }
        DialogState.stringMap.put(stringValue.toLowerCase(), this);
    }

}
