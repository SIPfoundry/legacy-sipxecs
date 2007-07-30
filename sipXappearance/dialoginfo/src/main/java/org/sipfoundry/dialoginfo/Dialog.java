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
  protected State state;

  protected long duration;

  protected DialogReplaces replaces;

  protected Nameaddr referredBy;

  protected Participant local;

  protected Participant remote;

  protected String id;

  protected String callId;

  protected String localTag;

  protected String remoteTag;

  protected String direction;

  protected ArrayList<String> routeSetList = new ArrayList<String>();


  public State getState() {
    return this.state;
  }

  public void setState(State state) {
    this.state = state;
  }

  public long getDuration() {
    return this.duration;
  }

  public void setDuration(long duration) {
    this.duration = duration;
  }

  public DialogReplaces getReplaces() {
    return this.replaces;
  }

  public void setReplaces(DialogReplaces replaces) {
    this.replaces = replaces;
  }

  public Nameaddr getReferredBy() {
    return this.referredBy;
  }

  public void setReferredBy(Nameaddr referredBy) {
    this.referredBy = referredBy;
  }

  public void addRouteSet(String routeSet) {
    routeSetList.add(routeSet);
  }

  public String getRouteSet(int index) {
    return (String)routeSetList.get( index );
  }

  public boolean validateRouteSet() {
    if (routeSetList.size() > 0) {
      return true;
    } else {
      return false;
    }
  }

  public int sizeRouteSetList() {
    return routeSetList.size();
  }

  public Participant getLocal() {
    return this.local;
  }

  public void setLocal(Participant local) {
    this.local = local;
  }

  public Participant getRemote() {
    return this.remote;
  }

  public void setRemote(Participant remote) {
    this.remote = remote;
  }

  public String getId() {
    return this.id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public String getCallId() {
    return this.callId;
  }

  public void setCallId(String callId) {
    this.callId = callId;
  }

  public String getLocalTag() {
    return this.localTag;
  }

  public void setLocalTag(String localTag) {
    this.localTag = localTag;
  }

  public String getRemoteTag() {
    return this.remoteTag;
  }

  public void setRemoteTag(String remoteTag) {
    this.remoteTag = remoteTag;
  }

  public String getDirection() {
    return this.direction;
  }

  public void setDirection(String direction) {
    this.direction = direction;
  }

}
