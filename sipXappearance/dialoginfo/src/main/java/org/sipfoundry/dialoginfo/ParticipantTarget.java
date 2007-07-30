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
public class ParticipantTarget {
  protected String uri;

  protected ArrayList<ParticipantTargetParam> paramList = new ArrayList<ParticipantTargetParam>();


  public void addParam(ParticipantTargetParam param) {
    paramList.add(param);
  }

  public ParticipantTargetParam getParam(int index) {
    return (ParticipantTargetParam)paramList.get( index );
  }

  public int sizeParamList() {
    return paramList.size();
  }

  public String getUri() {
    return this.uri;
  }

  public void setUri(String uri) {
    this.uri = uri;
  }

}
