/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.log4j;

import gov.nist.javax.sip.LogRecord;

public class SipFoundryLogRecord implements LogRecord {
    private String logRecord;

    public SipFoundryLogRecord(String logRecord) {
        this.logRecord = logRecord;
    }

    public String toString() {
        return logRecord;

    }

}
