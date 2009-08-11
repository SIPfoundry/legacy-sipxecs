/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

/**
 * Class representation of the participant element.
 * <p>
 *
 * @author Mardy Marshall
 */
public class Participant {
	/**
	 * The URI / display-name of the participant.
	 */
	protected Identity identity;

	/**
	 * The target URI.
	 */
	protected ParticipantTarget target;

	/**
	 * The session-description.
	 */
	protected SessionDescription sessionDescription;

	/**
	 * The cseq.
	 */
	protected long cseq;

	/**
	 * Retrieve the identity of the participant.
	 *
	 * @return The participant URI / display-name.
	 */
	public Identity getIdentity() {
		return identity;
	}

	/**
	 * Set the identity of the participant.
	 *
	 * @param identity
	 */
	public void setIdentity(Identity identity) {
		this.identity = identity;
	}

	/**
	 * Retrieve the target.
	 *
	 * @return The target.
	 */
	public ParticipantTarget getTarget() {
		return target;
	}

	/**
	 * Set the target.
	 *
	 * @param target
	 */
	public void setTarget(ParticipantTarget target) {
		this.target = target;
	}

	/**
	 * Retrieve the session-decription.
	 *
	 * @return The session-description.
	 */
	public SessionDescription getSessionDescription() {
		return sessionDescription;
	}

	/**
	 * Set the session-descriptioin.
	 *
	 * @param sessionDescription
	 */
	public void setSessionDescription(SessionDescription sessionDescription) {
		this.sessionDescription = sessionDescription;
	}

	/**
	 * Retrieve the cseq.
	 *
	 * @return The cseq.
	 */
	public long getCseq() {
		return cseq;
	}

	/**
	 * Set the cseq.
	 *
	 * @param cseq
	 */
	public void setCseq(long cseq) {
		this.cseq = cseq;
	}

}
