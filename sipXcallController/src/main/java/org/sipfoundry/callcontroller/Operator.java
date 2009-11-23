/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.callcontroller;

/**
 * This tracks the current operation for a given transaction.
 * 
 */
public enum Operator {
    SEND_NOTIFY, SEND_3PCC_REFER_CALL_SETUP, SEND_REFER,
    SEND_3PCC_CALL_SETUP1, SEND_3PCC_CALL_SETUP2, 
    SEND_3PCC_CALL_SETUP3, FORWARD_INVITE, SEND_OPTIONS, 
    FORWARD_REQUEST, SOLICIT_SDP_OFFER, SEND_SDP_OFFER;
}
