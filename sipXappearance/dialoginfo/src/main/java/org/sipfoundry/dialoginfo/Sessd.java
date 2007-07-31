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
public class Sessd {
	/**
	 * [Enter descriptive text here]
	 */
	private String element;

	/**
	 * [Enter descriptive text here]
	 */
	protected String type;

	/**
	 * JiBX private constructor.
	 */
	@SuppressWarnings("unused")
	private Sessd() {
		this.element = "";
	}

	/**
	 * Default constructor.
	 * 
	 * @param element
	 */
	public Sessd(String element) {
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
	public String getType() {
		return type;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param type
	 */
	public void setType(String type) {
		this.type = type;
	}

}
