/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

/**
 * Class representation of the dialog-replaces element.
 * <p>
 *
 * @author Mardy Marshall
 */
public class DialogReplaces {
	/**
	 * call-id of the initiating dialog.
	 */
	protected String callId;

	/**
	 * local-tag of the initiating dialog.
	 */
	protected String localTag;

	/**
	 * remote-tag of the initiating dialog.
	 */
	protected String remoteTag;

	/**
	 * Retrieve the call-id.
	 *
	 * @return The call-id.
	 */
	public String getCallId() {
		return callId;
	}

	/**
	 * Set the call-id.
	 *
	 * @param callId
	 */
	public void setCallId(String callId) {
		this.callId = callId;
	}

	/**
	 * Retrieve the local-tag.
	 *
	 * @return The local-tag.
	 */
	public String getLocalTag() {
		return localTag;
	}

	/**
	 * Set the local-tag.
	 *
	 * @param localTag
	 */
	public void setLocalTag(String localTag) {
		this.localTag = localTag;
	}

	/**
	 * Retrieve the remote-tag.
	 *
	 * @return The remote-tag.
	 */
	public String getRemoteTag() {
		return remoteTag;
	}

	/**
	 * Set the remote-tag.
	 *
	 * @param remoteTag
	 */
	public void setRemoteTag(String remoteTag) {
		this.remoteTag = remoteTag;
	}

}
