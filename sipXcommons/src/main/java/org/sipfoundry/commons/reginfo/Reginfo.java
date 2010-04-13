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
public class Reginfo {
	/**
	 * [Enter descriptive text here]
	 */
	protected long version;

	/**
	 * [Enter descriptive text here]
	 */
	protected String state;

	/**
	 * [Enter descriptive text here]
	 */
	protected ArrayList<Registration> registrationList = new ArrayList<Registration>();

	/**
	 * [Enter descriptive text here]
	 *
	 * @param registration
	 */
	public void addRegistration(Registration registration) {
		registrationList.add(registration);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param registration
	 */
	public void removeRegistration(Registration registration) {
		registrationList.remove(registration);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param index
	 * @return
	 */
	public Registration getRegistration(int index) {
		return (Registration) registrationList.get(index);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public int sizeRegistrationList() {
		return registrationList.size();
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public long getVersion() {
		return version;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param version
	 */
	public void setVersion(long version) {
		this.version = version;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public ReginfoState getState() {
		return ReginfoState.toEnum(state);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param state
	 */
	public void setState(ReginfoState state) {
		this.state = state.toString();
	}

}
