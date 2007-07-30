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
  protected Nameaddr identity;

  protected ParticipantTarget target;

  protected Sessd sessionDescription;

  protected long cseq;


  public Nameaddr getIdentity() {
    return this.identity;
  }

  public void setIdentity(Nameaddr identity) {
    this.identity = identity;
  }

  public ParticipantTarget getTarget() {
    return this.target;
  }

  public void setTarget(ParticipantTarget target) {
    this.target = target;
  }

  public Sessd getSessionDescription() {
    return this.sessionDescription;
  }

  public void setSessionDescription(Sessd sessionDescription) {
    this.sessionDescription = sessionDescription;
  }

  public long getCseq() {
    return this.cseq;
  }

  public void setCseq(long cseq) {
    this.cseq = cseq;
  }

}
