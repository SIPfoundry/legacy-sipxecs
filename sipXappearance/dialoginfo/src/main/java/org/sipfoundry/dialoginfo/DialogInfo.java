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
	/**
	 * [Enter descriptive text here]
	 */
	protected long version;

	/**
	 * [Enter descriptive text here]
	 */
	protected String state;

	/**
	 * [Enter descriptive text here]
	 */
	protected String entity;

	/**
	 * [Enter descriptive text here]
	 */
	protected ArrayList<Dialog> dialogList = new ArrayList<Dialog>();

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param dialog
	 */
	public void addDialog(Dialog dialog) {
		dialogList.add(dialog);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param dialog
	 */
	public void removeDialog(Dialog dialog) {
		dialogList.remove(dialog);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param index
	 * @return
	 */
	public Dialog getDialog(int index) {
		return (Dialog) dialogList.get(index);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public int sizeDialogList() {
		return dialogList.size();
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public long getVersion() {
		return version;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param version
	 */
	public void setVersion(long version) {
		this.version = version;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public DialogInfoState getState() {
		return DialogInfoState.toEnum(state);
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param state
	 */
	public void setState(DialogInfoState state) {
		this.state = state.toString();
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @return
	 */
	public String getEntity() {
		return entity;
	}

	/**
	 * [Enter descriptive text here]
	 * 
	 * @param entity
	 */
	public void setEntity(String entity) {
		this.entity = entity;
	}

}
