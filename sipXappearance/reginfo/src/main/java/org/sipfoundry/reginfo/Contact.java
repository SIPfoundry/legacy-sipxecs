/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.reginfo;

import java.util.ArrayList;


/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author mardy
 */
public class Contact {
  protected String uri;

  protected ContactDisplayName displayName;

  protected String state;

  protected String event;

  protected long durationRegistered;

  protected long expires;

  protected long retryAfter;

  protected String id;

  protected String q;

  protected String callid;

  protected long cseq;

  protected ArrayList<ContactUnknownParam> unknownParamList = new ArrayList<ContactUnknownParam>();


  public String getUri() {
    return this.uri;
  }

  public void setUri(String uri) {
    this.uri = uri;
  }

  public ContactDisplayName getDisplayName() {
    return this.displayName;
  }

  public void setDisplayName(ContactDisplayName displayName) {
    this.displayName = displayName;
  }

  public void addUnknownParam(ContactUnknownParam unknownParam) {
    unknownParamList.add(unknownParam);
  }

  public ContactUnknownParam getUnknownParam(int index) {
    return (ContactUnknownParam)unknownParamList.get( index );
  }

  public int sizeUnknownParamList() {
    return unknownParamList.size();
  }

  public String getState() {
    return this.state;
  }

  public void setState(String state) {
    this.state = state;
  }

  public String getEvent() {
    return this.event;
  }

  public void setEvent(String event) {
    this.event = event;
  }

  public long getDurationRegistered() {
    return this.durationRegistered;
  }

  public void setDurationRegistered(long durationRegistered) {
    this.durationRegistered = durationRegistered;
  }

  public long getExpires() {
    return this.expires;
  }

  public void setExpires(long expires) {
    this.expires = expires;
  }

  public long getRetryAfter() {
    return this.retryAfter;
  }

  public void setRetryAfter(long retryAfter) {
    this.retryAfter = retryAfter;
  }

  public String getId() {
    return this.id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public String getQ() {
    return this.q;
  }

  public void setQ(String q) {
    this.q = q;
  }

  public String getCallid() {
    return this.callid;
  }

  public void setCallid(String callid) {
    this.callid = callid;
  }

  public long getCseq() {
    return this.cseq;
  }

  public void setCseq(long cseq) {
    this.cseq = cseq;
  }

}
