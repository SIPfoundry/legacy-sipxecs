/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

/**
 * The class representation of the sessd element.
 * <p>
 *
 * @author Mardy Marshall
 */
public class SessionDescription {
	/**
	 * The sessd element.
	 */
	private String element;

	/**
	 * They type attribute.
	 */
	protected String type;

	/**
	 * JiBX private constructor.
	 */
	@SuppressWarnings("unused")
	private SessionDescription() {
	}

	/**
	 * Default constructor.
	 *
	 * @param element
	 */
	public SessionDescription(String element) {
		this.element = element;
	}

	/**
	 * Return the sessd element.
	 *
	 * @return The sessd.
	 */
	public String get() {
		return element;
	}

	/**
	 * Set the sessd element.
	 *
	 * @param element
	 */
	public void set(String element) {
		this.element = element;
	}

	/**
	 * Retrieve the type attribute.
	 *
	 * @return The type attribute.
	 */
	public String getType() {
		return type;
	}

	/**
	 * Set the type attribute.
	 *
	 * @param type
	 */
	public void setType(String type) {
		this.type = type;
	}

}
