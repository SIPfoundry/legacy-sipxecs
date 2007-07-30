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
public class Registration {
  protected String aor;

  protected String id;

  protected String state;

  protected ArrayList<Contact> contactList = new ArrayList<Contact>();


  public void addContact(Contact contact) {
    contactList.add(contact);
  }

  public Contact getContact(int index) {
    return (Contact)contactList.get(index);
  }

  public int sizeContactList() {
    return contactList.size();
  }

  public String getAor() {
    return this.aor;
  }

  public void setAor(String aor) {
    this.aor = aor;
  }

  public String getId() {
    return this.id;
  }

  public void setId(String id) {
    this.id = id;
  }

  public String getState() {
    return this.state;
  }

  public void setState(String state) {
    this.state = state;
  }

}
