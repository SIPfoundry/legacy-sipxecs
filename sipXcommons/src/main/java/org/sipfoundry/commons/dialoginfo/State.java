/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

/**
 * The class representation of the dialog state element.
 * <p>
 *
 * @author Mardy Marshall
 */
public class State {
	/**
	 * The state element.
	 */
	private String element;

	/**
	 * The event attribute.
	 */
	protected String event;

	/**
	 * The code attribute.
	 */
	protected long code;

	/**
	 * JiBX private constructor.
	 */
	@SuppressWarnings("unused")
	private State() {
	}

	/**
	 * Default constructor.
	 *
	 * @param dialogState
	 */
	public State(DialogState dialogState) {
		this.element = dialogState.toString();
	}

	/**
	 * Retrieve the dialog state.
	 *
	 * @return The state.
	 */
	public DialogState get() {
		return DialogState.toEnum(element);
	}

	/**
	 * Set the dialog state.
	 *
	 * @param dialogState
	 */
	public void set(DialogState dialogState) {
		this.element = dialogState.toString();
	}

	/**
	 * Retrieve the event attribute.
	 *
	 * @return The event attribute.
	 */
	public StateEvent getEvent() {
		return StateEvent.toEnum(event);
	}

	/**
	 * Set the event attribute.
	 *
	 * @param event
	 */
	public void setEvent(StateEvent event) {
		this.event = event.toString();
	}

	/**
	 * Retrieve the code attribute.
	 *
	 * @return The code attribute.
	 */
	public long getCode() {
		return code;
	}

	/**
	 * Set the code attribute.
	 *
	 * @param code
	 */
	public void setCode(long code) {
		this.code = code;
	}

}
