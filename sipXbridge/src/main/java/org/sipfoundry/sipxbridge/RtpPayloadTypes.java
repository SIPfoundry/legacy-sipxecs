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

    private static Hashtable<Integer,String> payloadTypeByInt = new Hashtable<Integer,String>();

    private static Logger logger = Logger.getLogger(RtpPayloadTypes.class);

    private static void putPayload(String name, int value) {
        payloadTypes.put(name, value);
        payloadTypeByInt.put(value, name);
    }

    static {
        putPayload("PCMU", 0);
        putPayload("G726",2);
        putPayload("GSM", 3);
        putPayload("G723", 4);
        putPayload("DVI4@8000h", 5);
        putPayload("DVI4@16000h",6);
        putPayload("LPC", 7);
        putPayload("PCMA", 8);
        putPayload("G722", 9);
        putPayload("L16_stereo",10);
        putPayload("L16",11);
        putPayload("QCELP", 12);
        putPayload("CN", 13);
        putPayload("MPA", 14);
        putPayload("G728", 15);
        putPayload("DVI4", 16);
        putPayload("G729", 18);
        putPayload("G726-32", 17); // Phoney


    }

    public static boolean isPayload(String payloadType) {
        return payloadTypes.containsKey(payloadType.toUpperCase());
    }

    public static boolean isPayload(int payloadType) {
        return payloadTypeByInt.containsKey(payloadType);

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

    public static String getPayloadType(int payloadType) {
        if ( payloadTypeByInt.containsKey(payloadType)) {
            return payloadTypeByInt.get(payloadType);
        } else {
            logger.warn("Cannot find payload type " + payloadType
                    + " returning 0");
            return null;
        }
    }

}
