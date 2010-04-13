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
public class Contact {
	/**
	 * [Enter descriptive text here]
	 */
	protected String uri;

	/**
	 * [Enter descriptive text here]
	 */
	protected ContactDisplayName displayName;

	/**
	 * [Enter descriptive text here]
	 */
	protected String state;

	/**
	 * [Enter descriptive text here]
	 */
	protected String event;

	/**
	 * [Enter descriptive text here]
	 */
	protected long durationRegistered;

	/**
	 * [Enter descriptive text here]
	 */
	protected long expires;

	/**
	 * [Enter descriptive text here]
	 */
	protected long retryAfter;

	/**
	 * [Enter descriptive text here]
	 */
	protected String id;

	/**
	 * [Enter descriptive text here]
	 */
	protected String q;

	/**
	 * [Enter descriptive text here]
	 */
	protected String callid;

	/**
	 * [Enter descriptive text here]
	 */
	protected long cseq;

	/**
	 * [Enter descriptive text here]
	 */
	protected ArrayList<ContactUnknownParam> unknownParamList = new ArrayList<ContactUnknownParam>();

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public String getUri() {
		return uri;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param uri
	 */
	public void setUri(String uri) {
		this.uri = uri;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public ContactDisplayName getDisplayName() {
		return displayName;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param displayName
	 */
	public void setDisplayName(ContactDisplayName displayName) {
		this.displayName = displayName;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param unknownParam
	 */
	public void addUnknownParam(ContactUnknownParam unknownParam) {
		unknownParamList.add(unknownParam);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param unknownParam
	 */
	public void removeUnknownParam(ContactUnknownParam unknownParam) {
		unknownParamList.remove(unknownParam);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param index
	 * @return
	 */
	public ContactUnknownParam getUnknownParam(int index) {
		return (ContactUnknownParam) unknownParamList.get(index);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public int sizeUnknownParamList() {
		return unknownParamList.size();
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public ContactState getState() {
		return ContactState.toEnum(state);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param state
	 */
	public void setState(ContactState state) {
		this.state = state.toString();
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public ContactEvent getEvent() {
		return ContactEvent.toEnum(event);
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param event
	 */
	public void setEvent(ContactEvent event) {
		this.event = event.toString();
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public long getDurationRegistered() {
		return durationRegistered;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param durationRegistered
	 */
	public void setDurationRegistered(long durationRegistered) {
		this.durationRegistered = durationRegistered;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public long getExpires() {
		return expires;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param expires
	 */
	public void setExpires(long expires) {
		this.expires = expires;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public long getRetryAfter() {
		return retryAfter;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param retryAfter
	 */
	public void setRetryAfter(long retryAfter) {
		this.retryAfter = retryAfter;
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
	public String getQ() {
		return q;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param q
	 */
	public void setQ(String q) {
		this.q = q;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @return
	 */
	public String getCallid() {
		return callid;
	}

	/**
	 * [Enter descriptive text here]
	 *
	 * @param callid
	 */
	public void setCallid(String callid) {
		this.callid = callid;
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
