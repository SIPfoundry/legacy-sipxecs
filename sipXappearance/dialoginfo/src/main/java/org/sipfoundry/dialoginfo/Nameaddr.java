/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.dialoginfo;

/**
 * [Enter descriptive text here]
 * <p>
 * 
 * @author mardy
 */
public class Nameaddr {
	/**
	 * [Enter descriptive text here]
	 */
	private String element;

	/**
	 * [Enter descriptive text here]
	 */
	protected String displayName;

	/**
	 * Default constructor
	 * 
	 * @param element
	 */
	public Nameaddr(String element) {
		this.element = element;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public String get() {
		return element;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param element
	 */
	public void set(String element) {
		this.element = element;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public String getDisplayName() {
		return displayName;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param displayName
	 */
	public void setDisplayName(String displayName) {
		this.displayName = displayName;
	}

}
