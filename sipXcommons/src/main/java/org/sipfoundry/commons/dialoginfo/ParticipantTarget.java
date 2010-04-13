/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

import java.util.ArrayList;
import java.util.Iterator;

/**
 * Class representation of the participant target.
 * <p>
 *
 * @author Mardy Marshall
 */
public class ParticipantTarget {
	/**
	 * The URI of the target.
	 */
	protected String uri;

	/**
	 * List of target parameters.
	 */
	protected ArrayList<ParticipantTargetParam> paramList = new ArrayList<ParticipantTargetParam>();

	/**
	 * Add a parameter to the target.
	 *
	 * @param param
	 */
	public void addParam(ParticipantTargetParam param) {
		paramList.add(param);
	}

	/**
	 * Remove the specified parameter from the target.
	 *
	 * @param param
	 */
	public void removeParam(ParticipantTargetParam param) {
		paramList.remove(param);
	}

	/**
	 * Retrieve the parameter at the specified index from the target.
	 *
	 * @param index
	 * @return The requested parameter.
	 */
	public ParticipantTargetParam getParam(int index) {
		return (ParticipantTargetParam) paramList.get(index);
	}

	/**
	 * Returns an iterator over the elements in this collection.
	 *
	 * @return  an <tt>Iterator</tt> over the elements in this collection
	 */
	public Iterator<ParticipantTargetParam> participantTargetParamListIterator() {
		return paramList.iterator();
	}

	/**
	 * Retrieve the number of parameters contained within the target.
	 *
	 * @return The number of parameters.
	 */
	public int sizeParamList() {
		return paramList.size();
	}

	/**
	 * Retrieve the URI of the target.
	 *
	 * @return The URI.
	 */
	public String getUri() {
		return uri;
	}

	/**
	 * Set the URI of the target.
	 *
	 * @param uri
	 */
	public void setUri(String uri) {
		this.uri = uri;
	}

}
