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
 */
public enum Operator {
    SEND_NOTIFY, SEND_3PCC_REFER_CALL_SETUP, SEND_REFER;
}
