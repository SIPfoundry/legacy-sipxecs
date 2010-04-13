/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

import java.util.concurrent.ConcurrentHashMap;

/**
 * Enumerations for dialog state event attribute.
 * <p>
 *
 * @author Mardy Marshall
 */
public enum StateEvent {
	/**
	 * The dialog state event is "cancelled".
	 */
	CANCELLED("cancelled"),

	/**
	 * The dialog state event is "rejected".
	 */
	REJECTED("rejected"),

	/**
	 * The dialog state event is "replaced".
	 */
	REPLACED("replaced"),

	/**
	 * The dialog state event is "local-bye".
	 */
	LOCAL_BYE("local-bye"),

	/**
	 * The dialog state event is "remote-bye".
	 */
	REMOTE_BYE("remote-bye"),

	/**
	 * The dialog state event is "error".
	 */
	ERROR("error"),

	/**
	 * The dialog state event is "timeout".
	 */
	TIMEOUT("timeout"),

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
	private static ConcurrentHashMap<String, StateEvent> stringMap;

	/**
	 * Default constructor.
	 *
	 * @param stringValue
	 *            String representation of enumeration.
	 */
	StateEvent(String stringValue) {
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
	public static StateEvent toEnum(String stringValue) {
		StateEvent value = StateEvent.stringMap.get(stringValue.toLowerCase());
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
        if (StateEvent.stringMap == null) {
            StateEvent.stringMap = new ConcurrentHashMap<String, StateEvent>();
        }
        StateEvent.stringMap.put(stringValue.toLowerCase(), this);
    }

}
