/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.sip.log4j;

import gov.nist.javax.sip.LogRecord;
import gov.nist.javax.sip.LogRecordFactory;

/**
 * This class returns a Log record for each SipMessage logged by the jain sip stack. It should
 * generate a log record with the correct format. This log format was obtained by reverse
 * engineering the log records found in the SIPX logs.
 *
 *
 */
public class SipFoundryLogRecordFactory implements LogRecordFactory {

    public static final String INCOMING = ":------->INCOMING<----------:";

    public static final String OUTGOING = ":------->OUTGOING<----------:";

    public static final String HOST = "----Remote Host:";

    public static final String PORT = "---- Port: ";

    public static final String OUTGOING_END = "--------------------END--------------------\n";

    public static final String INCOMING_END = "====================END====================\n";

    public static final String SIP_PORT = "5060";

    public static final String COLON = ":";

    public static final String TRAILING_DASH = "----\n";

    /**
     * Create a log record.
     *
     * @param message -- the message to be logged.
     * @param source -- host:port of the source of the message.
     * @param destination -- host:port of the destination of the message.
     * @param timeStamp -- The time at which this message was seen by the stack or sent out by the
     *        stack.
     * @param isSender -- true if we are sending the message false otherwise.
     * @param firstLine -- the first line of the message to be logged.
     * @param tid -- the transaction id
     * @param callId -- the call id
     * @param timestampVal -- the timestamp header value of the incoming message.
     *
     * @return -- a log record with the appropriate fields set.
     */

    public LogRecord createLogRecord(String message, String source, String destination, long timeStamp,
            boolean isSender, String firstLine, String tid, String callId, long timestampVal) {
        String[] sourceHostPort = source.split(COLON);
        String sourcePort = sourceHostPort.length == 1 ? SIP_PORT : sourceHostPort[1];
        String sourceHost = sourceHostPort[0];

        String[] destHostPort = destination.split(COLON);
        String destPort = destHostPort.length == 1 ? SIP_PORT : destHostPort[1];
        String destHost = destHostPort[0];
        String hostPort;
        if (!isSender) {
            hostPort = HOST + sourceHost + PORT + sourcePort + TRAILING_DASH;
        } else {
            hostPort = HOST + destHost + PORT + destPort + TRAILING_DASH;
        }
        String direction = isSender ? OUTGOING : INCOMING;
        String auxInfo = isSender ? "Sent SIP Message :\n" : "Read SIP Message :\n";
        String end = isSender ? OUTGOING_END : INCOMING_END;
        LogRecord logRecord = new SipFoundryLogRecord(direction + auxInfo + hostPort + message + end);
        return logRecord;
    }
}
