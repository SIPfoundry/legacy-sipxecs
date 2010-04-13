/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.reginfo;

import java.util.ArrayList;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class Registration {
	/**
	 * [Enter descriptive text here]
	 */
	protected String aor;

	/**
	 * [Enter descriptive text here]
	 */
	protected String id;

	/**
	 * [Enter descriptive text here]
	 */
	protected String state;

	/**
	 * [Enter descriptive text here]
	 */
	protected ArrayList<Contact> contactList = new ArrayList<Contact>();

	/**
	 * [Enter descriptive text here]
	 *
	 * @param contact
	 */
	public void addContact(Contact contact) {
		contactList.add(contact);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param contact
	 */
	public void removeContact(Contact contact) {
		contactList.remove(contact);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param index
	 * @return
	 */
	public Contact getContact(int index) {
		return (Contact) contactList.get(index);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public int sizeContactList() {
		return contactList.size();
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public String getAor() {
		return aor;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param aor
	 */
	public void setAor(String aor) {
		this.aor = aor;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public String getId() {
		return id;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param id
	 */
	public void setId(String id) {
		this.id = id;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public RegistrationState getState() {
		return RegistrationState.toEnum(state);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param state
	 */
	public void setState(RegistrationState state) {
		this.state = state.toString();
	}

}
