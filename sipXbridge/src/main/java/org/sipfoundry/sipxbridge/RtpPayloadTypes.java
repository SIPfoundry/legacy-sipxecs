/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.Hashtable;

import org.apache.log4j.Logger;

/**
 * A table of RTP payload types for early media.
 * 
 * @author M. Ranganathan
 * 
 */
public class RtpPayloadTypes {

    private static Hashtable<String, Integer> payloadTypes = new Hashtable<String, Integer>();

    private static Logger logger = Logger.getLogger(RtpPayloadTypes.class);

    static {
        payloadTypes.put("PCMU", 0);
        payloadTypes.put("GSM", 3);
        payloadTypes.put("G723", 4);
        payloadTypes.put("DVI4", 5);
        payloadTypes.put("LPC", 7);
        payloadTypes.put("PCMA", 8);
        payloadTypes.put("G722", 9);
        payloadTypes.put("QCELP", 12);
        payloadTypes.put("CN", 13);
        payloadTypes.put("MPA", 14);
        payloadTypes.put("G728", 15);
        payloadTypes.put("DVI4", 16);
        payloadTypes.put("G729", 18);

    }

    public static int getPayloadType(String payloadType) {
        if (payloadTypes.containsKey(payloadType.toUpperCase())) {
            return payloadTypes.get(payloadType.toUpperCase());
        } else {
            logger.warn("Cannot find payload type " + payloadType
                    + " returning 0");
            return 0;

        }
    }

}
