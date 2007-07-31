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
public class State {
	/**
	 * [Enter descriptive text here]
	 */
	private String element;

	/**
	 * [Enter descriptive text here]
	 */
	protected String event;

	/**
	 * [Enter descriptive text here]
	 */
	protected long code;

	/**
	 * JiBX private constructor.
	 */
	@SuppressWarnings("unused")
	private State() {
		this.element = "";
	}

	/**
	 * Default constructor.
	 * 
	 * @param element
	 */
	public State(DialogState dialogState) {
		this.element = dialogState.toString();
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public DialogState get() {
		return DialogState.toEnum(element);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param element
	 */
	public void set(DialogState dialogState) {
		this.element = dialogState.toString();
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public StateEvent getEvent() {
		return StateEvent.toEnum(event);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param event
	 */
	public void setEvent(StateEvent event) {
		this.event = event.toString();
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public long getCode() {
		return code;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param code
	 */
	public void setCode(long code) {
		this.code = code;
	}

}
