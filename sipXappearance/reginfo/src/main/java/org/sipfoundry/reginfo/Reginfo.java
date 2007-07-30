/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.reginfo;

import java.util.ArrayList;


public class Reginfo {
  protected long version;

  protected String state;

  protected ArrayList<Registration> registrationList = new ArrayList<Registration>();


  public void addRegistration(Registration registration) {
    registrationList.add(registration);
  }

  public Registration getRegistration(int index) {
    return (Registration)registrationList.get(index);
  }

  public int sizeRegistrationList() {
    return registrationList.size();
  }

  public long getVersion() {
    return this.version;
  }

  public void setVersion(long version) {
    this.version = version;
  }

  public String getState() {
    return this.state;
  }

  public void setState(String state) {
    this.state = state;
  }

}
