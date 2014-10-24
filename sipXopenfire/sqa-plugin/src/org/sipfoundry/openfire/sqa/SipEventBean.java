/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.openfire.sqa;

import ietf.params.xml.ns.dialog_info.Dialog;
import ietf.params.xml.ns.dialog_info.DialogInfo;
import ietf.params.xml.ns.dialog_info.Participant;
import ietf.params.xml.ns.dialog_info.State;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.commons.util.SipUriUtil;

public class SipEventBean {
	private DialogInfo m_dialogInfo;
	private String m_observerId;
	private String m_remoteId;
	private DialogDirection m_direction;
	private List<DialogElement> m_dialogElements;

	public SipEventBean(DialogInfo dialogInfo) {
		m_dialogInfo = dialogInfo;
		m_observerId = SipUriUtil.extractUserName(m_dialogInfo.getEntity());
		List<Dialog> dialogs = m_dialogInfo.getDialog();
		m_dialogElements = new ArrayList<DialogElement>();
		DialogElement element = null;
		for (Dialog dialog : dialogs) {
			State state = dialog.getState();
			String direction = dialog.getDirection();
			String id = dialog.getId();
			DialogDirection dialogDirection = direction != null ? DialogDirection.getEnum(direction) : null;
			if (m_direction == null && dialogDirection != null) {
			    m_direction = dialogDirection;
			}
			DialogState dialogState = state != null ? DialogState.getEnum(state.getValue()) : null;
			DialogStateEvent dialogEvent = state != null ? DialogStateEvent.getEnum(state.getEvent()) : null;

			element = new DialogElement(id, dialogDirection, dialogState, dialogEvent);

			Participant participant = dialog.getRemote();
			if (m_remoteId == null) {
				m_remoteId = participant != null ? participant.getIdentity() != null ? participant.getIdentity().getValue() : null : null;
				if (m_remoteId != null) {
				    m_remoteId = SipUriUtil.extractUserName(m_remoteId);
				}
			}
			m_dialogElements.add(element);
		}
	}

	public boolean isOnHook() {
		return m_dialogElements.isEmpty();
	}

	public boolean isTrying() {
	    return isState(DialogState.trying);
	}

	public boolean isConfirmed() {
	    return isState(DialogState.confirmed);
	}

	public boolean isTerminated() {
	    return isState(DialogState.terminated);
	}

	public boolean isRinging() {
	    return isState(DialogState.early);
	}

	public String getCallerId() {
	    return (m_direction != null && m_direction.equals(DialogDirection.initiator)) ? m_observerId : m_remoteId;
	}

	public String getCalleeId() {
	    return (m_direction != null && m_direction.equals(DialogDirection.recipient)) ? m_observerId : m_remoteId;
	}

	private boolean isState(DialogState state) {
        for (DialogElement element : m_dialogElements) {
            if(element.getDialogState().equals(state)) {
                return true;
            }
        }
        return false;
	}

	private String getDialogIdWithState(DialogState state) {
        for (DialogElement element : m_dialogElements) {
            if(element.getDialogState().equals(state)) {
                return element.getId();
            }
        }
        return null;
	}

	public String getConfirmedDialogId() {
	    return getDialogIdWithState(DialogState.confirmed);
	}

	public String getTerminatedDialogId() {
	    return getDialogIdWithState(DialogState.terminated);
	}

	public String getObserverId() {
		return m_observerId;
	}

	public static final class DialogDirection extends Enum {
        public static final DialogDirection initiator = new DialogDirection("initiator");
        public static final DialogDirection recipient = new DialogDirection("recipient");

        public DialogDirection(String name) {
            super(name);
        }
        public static DialogDirection getEnum(String type) {
            return (DialogDirection) getEnum(DialogDirection.class, type);
        }
    }

	public static final class DialogState extends Enum {
        public static final DialogState trying = new DialogState("trying");
        public static final DialogState proceeding = new DialogState("proceeding");
        public static final DialogState early = new DialogState("early");
        public static final DialogState terminated = new DialogState("terminated");
        public static final DialogState confirmed = new DialogState("confirmed");

        public DialogState(String name) {
            super(name);
        }
        public static DialogState getEnum(String type) {
            return (DialogState) getEnum(DialogState.class, type);
        }
    }

	public static final class DialogStateEvent extends Enum {
        public static final DialogStateEvent cancelled = new DialogStateEvent("cancelled");
        public static final DialogStateEvent rejected = new DialogStateEvent("rejected");
        public static final DialogStateEvent replaced = new DialogStateEvent("replaced");
        public static final DialogStateEvent local_bye = new DialogStateEvent("local-bye");
        public static final DialogStateEvent remote_bye = new DialogStateEvent("remote-bye");
        public static final DialogStateEvent error = new DialogStateEvent("error");
        public static final DialogStateEvent timeout = new DialogStateEvent("timeout");

        public DialogStateEvent(String name) {
            super(name);
        }
        public static DialogStateEvent getEnum(String type) {
            return (DialogStateEvent) getEnum(DialogStateEvent.class, type);
        }
    }

	public static final class DialogElement {
	    private String m_id;
		private DialogDirection m_dialogDirection;
		private DialogState m_dialogState;
		private DialogStateEvent m_dialogStateEvent;
		public DialogElement(String id, DialogDirection dialogDirection,
				DialogState dialogState, DialogStateEvent dialogStateEvent) {
			m_dialogDirection = dialogDirection;
			m_dialogState = dialogState;
			m_dialogStateEvent = dialogStateEvent;
			m_id = id;
		}
		public DialogDirection getDialogDirection() {
			return m_dialogDirection;
		}
		public DialogState getDialogState() {
			return m_dialogState;
		}
		public DialogStateEvent getDialogStateEvent() {
			return m_dialogStateEvent;
		}
		public String getId() {
		    return m_id;
		}
	}
}
