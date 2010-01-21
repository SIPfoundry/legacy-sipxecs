/*
 *  Copyright (C) 2008 Nortel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

/**
 * The action that specifies what to do with the current dialog.
 * This action is stored in the DialogContext.
 * 
 * @author M. Ranganathan
 */
public enum PendingDialogAction {
	
	PENDING_RE_INVITE_WITH_SDP_OFFER, PENDING_SDP_ANSWER_IN_ACK,
	PENDING_FORWARD_ACK_WITH_SDP_ANSWER, NONE, TERMINATE_ON_CONFIRM,
	PENDING_SOLICIT_SDP_OFFER_ON_ACK, PENDING_RE_INVITE_REMOVE_RELAY;

}
