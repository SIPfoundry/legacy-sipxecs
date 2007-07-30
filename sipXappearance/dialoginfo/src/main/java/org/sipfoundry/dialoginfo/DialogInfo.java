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
public class DialogInfo {
  protected long version;

  protected String state;

  protected String entity;

  protected ArrayList<Dialog> dialogList = new ArrayList<Dialog>();


  public void addDialog(Dialog dialog) {
    dialogList.add(dialog);
  }

  public Dialog getDialog(int index) {
    return (Dialog)dialogList.get( index );
  }

  public int sizeDialogList() {
    return dialogList.size();
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

  public String getEntity() {
    return this.entity;
  }

  public void setEntity(String entity) {
    this.entity = entity;
  }

}
