/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxbridge;

import javax.sip.Dialog;
import javax.sip.RequestEvent;

/**
 * Temporary holding place for context data.
 */
class ForwardSdpSolicitationContinuationData implements ContinuationData {
	private RequestEvent requestEvent;

	ForwardSdpSolicitationContinuationData ( RequestEvent requestEvent ) {
		this.requestEvent = requestEvent;
	}

	public Dialog getDialog() {
		
		return requestEvent.getDialog();
	}

	public Operation getOperation() {
		
		return Operation.FORWARD_SDP_SOLICITIATION;
	}

	public RequestEvent getRequestEvent() {
		return requestEvent;
	}

}
