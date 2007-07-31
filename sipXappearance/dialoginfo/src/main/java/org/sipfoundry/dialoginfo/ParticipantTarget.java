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
	/**
	 * [Enter descriptive text here]
	 */
	protected String uri;

	/**
	 * [Enter descriptive text here]
	 */
	protected ArrayList<ParticipantTargetParam> paramList = new ArrayList<ParticipantTargetParam>();

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param param
	 */
	public void addParam(ParticipantTargetParam param) {
		paramList.add(param);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param param
	 */
	public void removeParam(ParticipantTargetParam param) {
		paramList.remove(param);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param index
	 * @return
	 */
	public ParticipantTargetParam getParam(int index) {
		return (ParticipantTargetParam) paramList.get(index);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public int sizeParamList() {
		return paramList.size();
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public String getUri() {
		return uri;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param uri
	 */
	public void setUri(String uri) {
		this.uri = uri;
	}

}
