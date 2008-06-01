/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.sip;

/**
 * This tracks the current operation for a given transaction.
 * 
 * @author mranga
 * 
 */
public enum Operator {
    SEND_NOTIFY, SEND_3PCC_CALL_SETUP;
}
