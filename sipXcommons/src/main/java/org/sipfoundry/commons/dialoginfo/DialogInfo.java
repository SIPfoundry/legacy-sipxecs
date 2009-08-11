/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.dialoginfo;

import java.util.ArrayList;

/**
 * Top level class representation of the dialog-info element.
 * <p>
 * UML class diagram:
 * <p>
 * <img src="doc-files/dialoginfo.jpg">
 *
 * @author Mardy Marshall
 */
public class DialogInfo {
	/**
	 * Version number of dialog-info notification.
	 */
	protected long version;

	/**
	 * State of dialog-info notification, either "full" or "partial".
	 */
	protected String state;

	/**
	 * The URI associated with the dialog-info notification.
	 */
	protected String entity;

	/**
	 * List of dialogs represented by the dialog-info notification.
	 */
	protected ArrayList<Dialog> dialogList = new ArrayList<Dialog>();

	/**
	 * Add a Dialog instance to the notification.
	 *
	 * @param dialog
	 */
	public void addDialog(Dialog dialog) {
		dialogList.add(dialog);
	}

	/**
	 * Remove the specified Dialog instance from the notification.
	 *
	 * @param dialog
	 */
	public void removeDialog(Dialog dialog) {
		dialogList.remove(dialog);
	}

	/**
	 * Retrieve a Dialog instance at the specified index.
	 *
	 * @param index
	 * @return Dialog instance.
	 */
	public Dialog getDialog(int index) {
		return (Dialog) dialogList.get(index);
	}

	/**
	 * Return the number of Dialog instances contained.
	 *
	 * @return Number of Dialog instances.
	 */
	public int sizeDialogList() {
		return dialogList.size();
	}

	/**
	 * Retrieve the version number of the notification
	 *
	 * @return Version number.
	 */
	public long getVersion() {
		return version;
	}

	/**
	 * Set the version number of the notification
	 *
	 * @param version
	 */
	public void setVersion(long version) {
		this.version = version;
	}

	/**
	 * Retrieve the state of the notification
	 *
	 * @return DialogInfoState enumeration representing the state.
	 */
	public DialogInfoState getState() {
		return DialogInfoState.toEnum(state);
	}

	/**
	 * Set the state of the notification.
	 *
	 * @param state
	 */
	public void setState(DialogInfoState state) {
		this.state = state.toString();
	}

	/**
	 * Retrieve the URI associated with the notification.
	 *
	 * @return Entity URI.
	 */
	public String getEntity() {
		return entity;
	}

	/**
	 * Set the URI associated with the notification.
	 *
	 * @param entity
	 */
	public void setEntity(String entity) {
		this.entity = entity;
	}

}
