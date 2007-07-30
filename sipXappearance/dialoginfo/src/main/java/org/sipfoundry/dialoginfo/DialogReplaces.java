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
public class DialogReplaces {
  protected String callId;

  protected String localTag;

  protected String remoteTag;


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

}
