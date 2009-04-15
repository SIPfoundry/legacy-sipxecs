/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.fswitchtester;

import java.text.SimpleDateFormat;
import java.util.TimeZone;

import org.apache.log4j.Layout;
import org.apache.log4j.Level;
import org.apache.log4j.Priority;
import org.apache.log4j.spi.LoggingEvent;



/**
 * A log4j Layout class that matches the SipFoundry C++ OsSyslog format (within
 * reason)
 * 
 * @author Woof!
 */
public class SipFoundryLayout extends Layout {
    static Long lineNumber = 0L;
    SimpleDateFormat dateFormat;
    String hostName;
    String facility;

    /**
     * Sorry for this ugly routine. It is here because SIPX needs six digits 
     * and I cannot find a way to make SimpleDateFormat do that.
     */
    private String munge(String input) {
        String[] pieces = input.split("\\.");
        if ( pieces.length == 0 ) return input;
        StringBuffer newStringBuilder = new StringBuffer().append(input);
       
        if ( pieces[pieces.length -1 ].length() == 4 ) {
            newStringBuilder.deleteCharAt(input.length()  -1);
            newStringBuilder.append("000Z");
            return newStringBuilder.toString();
        } else {
            return input;
        }
    }
    
    public SipFoundryLayout() {
        super();
        dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS'Z'");
        dateFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
        hostName = Hostname.get();
        facility = "JAVA"; // Can be set from the property
        // log4j.appender.xxx.layout.facility
    }

    /**
     * Map the log4j levels to the SipFoundry text (ERROR is ERR)
     * 
     * @param l
     *            The level to map
     * @return The SipFoundry level text if available, otherwise the log4j text
     */
    private String mapLevel2SipFoundry(Level l) {
        switch (l.toInt()) {
        case Priority.DEBUG_INT:
            return "DEBUG";
        case Priority.INFO_INT:
            return "INFO";
        case Priority.WARN_INT:
            return "WARNING";
        case Priority.ERROR_INT:
            return "ERR";
        default:
            return l.toString();
        }
    }

    /**
     * Map the SipFoundry text to the log4j Priority number
     * 
     * @param level
     * @return
     */
    public static Level mapSipFoundry2log4j(String level) {
        if (level == null)
            return Level.DEBUG;

        if (level.equalsIgnoreCase("DEBUG"))
            return Level.DEBUG;
        if (level.equalsIgnoreCase("INFO"))
            return Level.INFO;
        if (level.equalsIgnoreCase("NOTICE"))
            return Level.INFO;
        if (level.equalsIgnoreCase("WARNING"))
            return Level.WARN;
        if (level.equalsIgnoreCase("ERR"))
            return Level.ERROR;
        if (level.equalsIgnoreCase("ERROR"))
            return Level.ERROR;

        Level l = Level.toLevel(level);
        return l == null ? Level.DEBUG : l;
    }

    /**
     * Escape any CR or LF in the message with the \r \n escapes. SipFoundry
     * logging logs multiline messages (like a SIP PDU) on a single log entry by
     * escaping the CRs and LFs
     * 
     * @param msg
     *            The message to escape
     * @return The escaped message
     */
    String escapeCrlf(String msg) {
        if (msg == null) {
            return null;
        }

        int n = msg.length();

        // Ignore trailing CR LFs (why?)
        /*
         * for(int i=n-1; i>0; i--) { char c = msg.charAt(i) ; if (c == '\r' ||
         * c == '\n') { n-- ; continue ; } break ; }
         */

        // escape CR LFs
        StringBuffer sb = new StringBuffer(n + 2);
        for (int i = 0; i < n; i++) {
            char c = msg.charAt(i);
            if (c == '\r') {
                sb.append("\\r");
            } else if (c == '\n') {
                sb.append("\\n");
            } else {
                sb.append(c);
            }
        }

        return sb.toString();
    }

    @Override
    public String format(LoggingEvent arg0) {
        String msg = escapeCrlf(arg0.getRenderedMessage());
        if ( msg == null ) return "";
        String loggerNames[] = arg0.getLoggerName().split("[.]");
        String loggerName = loggerNames[loggerNames.length - 1];

        // syslog2siptrace needs these facilities, and this
        // is a cheap hack to get them!
        // Actually, syslog2siptrace needs to know ip addrs and ports from
        // the messages, and that info ain't there at the moment. So just
        // ignore this for now...
        //
        String localFacility = facility;
        String newMessage = msg;

        if (msg.contains(SipFoundryLogRecordFactory.OUTGOING)) {
            localFacility = "OUTGOING";
            newMessage = msg.replaceFirst(SipFoundryLogRecordFactory.OUTGOING,
                    "");
        } else if (msg.contains(SipFoundryLogRecordFactory.INCOMING)) {
            localFacility = "INCOMING";
            newMessage = msg.replaceFirst(SipFoundryLogRecordFactory.INCOMING,
                    "");
        }

        // lineNumber is static across all loggers, so must be mutex protected.
        // time should also increase monotonically, so hold the lock
        synchronized (lineNumber) {
            lineNumber++;
            String out1 = String.format("\"%s\":%d:%s:%s:%s:%s:%s:%s:\"%s\"%n",
                    munge(dateFormat.format(System.currentTimeMillis())), lineNumber, // line
                    // number
                    localFacility, // Facility
                    mapLevel2SipFoundry(arg0.getLevel()), // msg priority
                    // (DEBUG, WARN,
                    // etc.)
                    hostName, // Name of this machine
                    arg0.getThreadName(), // Thread that called log
                    "00000000", // Thread Id (not useful in Java)
                    loggerName, // Name of the logger
                    newMessage); // The message itself (w CRLF escaped)
            return out1;
        }
    }

    @Override
    public boolean ignoresThrowable() {
        return true;
    }

   
    public void activateOptions() {
        // No options
        return;
    }

    public void setFacility(String facility) {
        this.facility = facility;
    }

    public String getFacility() {
        return facility;
    }
}
