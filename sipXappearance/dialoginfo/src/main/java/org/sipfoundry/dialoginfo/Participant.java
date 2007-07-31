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
public class Participant {
	/**
	 * [Enter descriptive text here]
	 */
	protected Nameaddr identity;

	/**
	 * [Enter descriptive text here]
	 */
	protected ParticipantTarget target;

	/**
	 * [Enter descriptive text here]
	 */
	protected Sessd sessionDescription;

	/**
	 * [Enter descriptive text here]
	 */
	protected long cseq;

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public Nameaddr getIdentity() {
		return identity;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param identity
	 */
	public void setIdentity(Nameaddr identity) {
		this.identity = identity;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public ParticipantTarget getTarget() {
		return target;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param target
	 */
	public void setTarget(ParticipantTarget target) {
		this.target = target;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public Sessd getSessionDescription() {
		return sessionDescription;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param sessionDescription
	 */
	public void setSessionDescription(Sessd sessionDescription) {
		this.sessionDescription = sessionDescription;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public long getCseq() {
		return cseq;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param cseq
	 */
	public void setCseq(long cseq) {
		this.cseq = cseq;
	}

}
