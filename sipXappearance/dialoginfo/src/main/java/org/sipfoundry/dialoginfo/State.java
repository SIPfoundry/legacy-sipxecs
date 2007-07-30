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
public class State {
  private String base;

  protected String event;

  protected long code;


  public String getBase() {
    return this.base;
  }

  public void setBase(String base) {
    this.base = base;
  }

  public String getEvent() {
    return this.event;
  }

  public void setEvent(String event) {
    this.event = event;
  }

  public long getCode() {
    return this.code;
  }

  public void setCode(long code) {
    this.code = code;
  }

}
