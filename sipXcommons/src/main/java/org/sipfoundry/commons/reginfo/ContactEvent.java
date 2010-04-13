/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.reginfo;

import java.util.concurrent.ConcurrentHashMap;

/**
 * Enumerations for contact registration event attribute.
 * <p>
 *
 * @author Mardy Marshall
 */
public enum ContactEvent {
	/**
	 * The contact is registered.
	 */
	REGISTERED("registered"),

	/**
	 * The contact registration has been created.
	 */
	CREATED("created"),

	/**
	 * The contact registration has been refreshed.
	 */
	REFRESHED("refreshed"),

	/**
	 * The contact registration period has been shortened.
	 */
	SHORTENED("shortened"),

	/**
	 * The contact registration has expired.
	 */
	EXPIRED("expired"),

	/**
	 * The contact registration has been deactivated.
	 */
	DEACTIVATED("deactivated"),

	/**
	 * The contact is on probation.
	 */
	PROBATION("probation"),

	/**
	 * The contact is unregistered.
	 */
	UNREGISTERED("unregistered"),

	/**
	 * The contact registration has been rejected.
	 */
	REJECTED("rejected"),

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
	private static ConcurrentHashMap<String, ContactEvent> stringMap;

	/**
	 * Default constructor.
	 *
	 * @param stringValue
	 *            String representation of enumeration.
	 */
	ContactEvent(String stringValue) {
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
	public static ContactEvent toEnum(String stringValue) {
		ContactEvent value = ContactEvent.stringMap.get(stringValue.toLowerCase());
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
		if (ContactEvent.stringMap == null) {
			ContactEvent.stringMap = new ConcurrentHashMap<String, ContactEvent>();
		}
		ContactEvent.stringMap.put(stringValue.toLowerCase(), this);
	}
}
