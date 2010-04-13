/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

/**
 * Class representation of the nameaddr element.
 * <p>
 *
 * @author Mardy Marshall
 */
public class Identity {
	/**
	 * The URI element.
	 */
	private String element;

	/**
	 * The display-name attribute.
	 */
	protected String displayName;

	/**
	 * Default constructor
	 *
	 */
	public Identity() {
		this.element = null;
	}

	/**
	 * Constructor which specifies identity.
	 *
	 * @param uri
	 */
	public Identity(String uri) {
		this.element = uri;
	}

	/**
	 * Retrieve the URI element.
	 *
	 * @return The URI.
	 */
	public String get() {
		return element;
	}

	/**
	 * Set the URI element.
	 *
	 * @param element
	 */
	public void set(String element) {
		this.element = element;
	}

	/**
	 * Retrieve the display-name attribute.
	 *
	 * @return The display-name.
	 */
	public String getDisplayName() {
		return displayName;
	}

	/**
	 * Set the display-name attribute.
	 *
	 * @param displayName
	 */
	public void setDisplayName(String displayName) {
		this.displayName = displayName;
	}

}
