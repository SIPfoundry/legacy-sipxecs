/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

/**
 * The class representation of the participant target param.
 * <p>
 *
 * @author Mardy Marshall
 */
public class ParticipantTargetParam {
	/**
	 * The pname attribute.
	 */
	protected String pname;

	/**
	 * The pval attribute.
	 */
	protected String pval;

	/**
	 * JiBX private constructor.
	 */
	@SuppressWarnings("unused")
	private ParticipantTargetParam() {
	}

	/**
	 * Default constructor.
	 *
	 * @param pname
	 * @param pval
	 */
	public ParticipantTargetParam(String pname, String pval) {
		this.pname = pname;
		this.pval = pval;
	}

	/**
	 * Retrieve the pname attribute.
	 *
	 * @return The pname attribute.
	 */
	public String getPname() {
		return pname;
	}

	/**
	 * Set the pname attribute.
	 *
	 * @param pname
	 */
	public void setPname(String pname) {
		this.pname = pname;
	}

	/**
	 * Retrieve the pval attribute.
	 *
	 * @return The pval attribute.
	 */
	public String getPval() {
		return pval;
	}

	/**
	 * Set the pval attribute.
	 *
	 * @param pval
	 */
	public void setPval(String pval) {
		this.pval = pval;
	}

}
