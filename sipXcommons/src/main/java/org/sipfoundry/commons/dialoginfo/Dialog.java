/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

import java.util.ArrayList;

/**
 * Class representation of the dialog element.
 * <p>
 *
 * @author Mardy Marshall
 */
public class Dialog {
	/**
	 * The state of the dialog.
	 */
	protected State state;

	/**
	 * The time in seconds since the associated dialog FSM was created.
	 */
	protected long duration;

	/**
	 * The dialog that this one is replacing.
	 */
	protected DialogReplaces replaces;

	/**
	 * Correlation to initiating REFER request.
	 */
	protected Identity referredBy;

	/**
	 * Local element associated with this dialog.
	 */
	protected Participant local;

	/**
	 * Remote element associated with this dialog.
	 */
	protected Participant remote;

	/**
	 * Unique identifier for this dialog.
	 */
	protected String id;

	/**
	 * The call-id of this dialog.
	 */
	protected String callId;

	/**
	 * The local-tag of this dialog.
	 */
	protected String localTag;

	/**
	 * The remote-tag of the dialog.
	 */
	protected String remoteTag;

	/**
	 * Direction of the dialog, either "initiator" or "recipient".
	 */
	protected String direction;

	/**
	 * List of associated route-set hop's.
	 */
	protected ArrayList<String> routeSetList = new ArrayList<String>();

	/**
	 * Retrieve the state of the dialog.
	 *
	 * @return The state of the dialog.
	 */
	public State getState() {
		return state;
	}

	/**
	 * Set the state of the dialog.
	 *
	 * @param state
	 */
	public void setState(State state) {
		this.state = state;
	}

	/**
	 * Retrieve the duration of the dialog.
	 *
	 * @return The duration, in seconds, of the dialog.
	 */
	public long getDuration() {
		return duration;
	}

	/**
	 * Set the duration of the dialog.
	 *
	 * @param duration
	 */
	public void setDuration(long duration) {
		this.duration = duration;
	}

	/**
	 * Retrieve the replaces element of the dialog.
	 *
	 * @return replaces element as a DialogReplaces instance.
	 */
	public DialogReplaces getReplaces() {
		return replaces;
	}

	/**
	 * Set the replaces element of the dialog.
	 *
	 * @param replaces
	 */
	public void setReplaces(DialogReplaces replaces) {
		this.replaces = replaces;
	}

	/**
	 * Retrieves the refered-by element of the dialog.
	 *
	 * @return refered-by element as a Nameaddr instance.
	 */
	public Identity getReferredBy() {
		return referredBy;
	}

	/**
	 * Set the refered-by element of the dialog.
	 *
	 * @param referredBy
	 */
	public void setReferredBy(Identity referredBy) {
		this.referredBy = referredBy;
	}

	/**
	 * Add a hop to the route-set of the dialog.
	 *
	 * @param hop
	 */
	public void addRouteSetHop(String hop) {
		routeSetList.add(hop);
	}

	/**
	 * Remove a hop from the route-set of the dialog.
	 *
	 * @param hop
	 */
	public void removeRouteSetHop(String hop) {
		routeSetList.remove(hop);
	}

	/**
	 * Retrieve a hop at the specified index from the route-set.
	 *
	 * @param index
	 * @return The requested hop.
	 */
	public String getRouteSetHop(int index) {
		return (String) routeSetList.get(index);
	}

	/**
	 * JiBX test for non-empty route-set.
	 *
	 * @return True if one or more hops in route-set.
	 */
	public boolean validateRouteSet() {
		if (routeSetList.size() > 0) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Return the number of hops in the route-set.
	 *
	 * @return The number of hops in the route-set.
	 */
	public int sizeRouteSetList() {
		return routeSetList.size();
	}

	/**
	 * Retrieve the local participant of the dialog.
	 *
	 * @return The local participant as an instance of Participant.
	 */
	public Participant getLocal() {
		return local;
	}

	/**
	 * Set the local participant of the dialog.
	 *
	 * @param local
	 */
	public void setLocal(Participant local) {
		this.local = local;
	}

	/**
	 * Retrieve the remote participant of the dialog.
	 *
	 * @return The remote participant as an instance of Participant.
	 */
	public Participant getRemote() {
		return remote;
	}

	/**
	 * Set the remote participant of the dialog.
	 *
	 * @param remote
	 */
	public void setRemote(Participant remote) {
		this.remote = remote;
	}

	/**
	 * Retrieve the id of the dialog.
	 *
	 * @return The id of the dialog.
	 */
	public String getId() {
		return id;
	}

	/**
	 * Set the id of the dialog.
	 *
	 * @param id
	 */
	public void setId(String id) {
		this.id = id;
	}

	/**
	 * Retrieve the associated call-id of the dialog.
	 *
	 * @return The call-id of the dialog.
	 */
	public String getCallId() {
		return callId;
	}

	/**
	 * Set the call-id of the dialog.
	 *
	 * @param callId
	 */
	public void setCallId(String callId) {
		this.callId = callId;
	}

	/**
	 * Retrieve the local-tag of the dialog.
	 *
	 * @return The local-tag of the dialog.
	 */
	public String getLocalTag() {
		return localTag;
	}

	/**
	 * Set the local-tag of the dialog.
	 *
	 * @param localTag
	 */
	public void setLocalTag(String localTag) {
		this.localTag = localTag;
	}

	/**
	 * Retrieve the remote-tag of the dialog.
	 *
	 * @return The remote-tag of the dialog.
	 */
	public String getRemoteTag() {
		return remoteTag;
	}

	/**
	 * Set the remote-tag of the dialog.
	 *
	 * @param remoteTag
	 */
	public void setRemoteTag(String remoteTag) {
		this.remoteTag = remoteTag;
	}

	/**
	 * Retrieve the direction of the dialog.
	 *
	 * @return The direction of the dialog.
	 */
	public DialogDirection getDirection() {
		return DialogDirection.toEnum(direction);
	}

	/**
	 * Set the direction of the dialog.
	 *
	 * @param direction
	 */
	public void setDirection(DialogDirection direction) {
		this.direction = direction.toString();
	}

}
