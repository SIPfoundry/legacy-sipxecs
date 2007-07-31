/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.dialoginfo;

import java.util.ArrayList;

/**
 * [Enter descriptive text here]
 * <p>
 * 
 * @author mardy
 */
public class Dialog {
	/**
	 * [Enter descriptive text here]
	 */
	protected State state;

	/**
	 * [Enter descriptive text here]
	 */
	protected long duration;

	/**
	 * [Enter descriptive text here]
	 */
	protected DialogReplaces replaces;

	/**
	 * [Enter descriptive text here]
	 */
	protected Nameaddr referredBy;

	/**
	 * [Enter descriptive text here]
	 */
	protected Participant local;

	/**
	 * [Enter descriptive text here]
	 */
	protected Participant remote;

	/**
	 * [Enter descriptive text here]
	 */
	protected String id;

	/**
	 * [Enter descriptive text here]
	 */
	protected String callId;

	/**
	 * [Enter descriptive text here]
	 */
	protected String localTag;

	/**
	 * [Enter descriptive text here]
	 */
	protected String remoteTag;

	/**
	 * [Enter descriptive text here]
	 */
	protected String direction;

	/**
	 * [Enter descriptive text here]
	 */
	protected ArrayList<String> routeSetList = new ArrayList<String>();

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public State getState() {
		return state;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param state
	 */
	public void setState(State state) {
		this.state = state;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public long getDuration() {
		return duration;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param duration
	 */
	public void setDuration(long duration) {
		this.duration = duration;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public DialogReplaces getReplaces() {
		return replaces;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param replaces
	 */
	public void setReplaces(DialogReplaces replaces) {
		this.replaces = replaces;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public Nameaddr getReferredBy() {
		return referredBy;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param referredBy
	 */
	public void setReferredBy(Nameaddr referredBy) {
		this.referredBy = referredBy;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param routeSet
	 */
	public void addRouteSet(String routeSet) {
		routeSetList.add(routeSet);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param routeSet
	 */
	public void removeRouteSet(String routeSet) {
		routeSetList.remove(routeSet);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param index
	 * @return
	 */
	public String getRouteSet(int index) {
		return (String) routeSetList.get(index);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public boolean validateRouteSet() {
		if (routeSetList.size() > 0) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public int sizeRouteSetList() {
		return routeSetList.size();
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public Participant getLocal() {
		return local;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param local
	 */
	public void setLocal(Participant local) {
		this.local = local;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public Participant getRemote() {
		return remote;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param remote
	 */
	public void setRemote(Participant remote) {
		this.remote = remote;
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
	public String getCallId() {
		return callId;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param callId
	 */
	public void setCallId(String callId) {
		this.callId = callId;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public String getLocalTag() {
		return localTag;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param localTag
	 */
	public void setLocalTag(String localTag) {
		this.localTag = localTag;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public String getRemoteTag() {
		return remoteTag;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param remoteTag
	 */
	public void setRemoteTag(String remoteTag) {
		this.remoteTag = remoteTag;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public DialogDirection getDirection() {
		return DialogDirection.toEnum(direction);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param direction
	 */
	public void setDirection(DialogDirection direction) {
		this.direction = direction.toString();
	}

}
