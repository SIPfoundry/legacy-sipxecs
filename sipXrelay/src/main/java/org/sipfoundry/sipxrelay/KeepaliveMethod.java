/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxrelay;

public enum KeepaliveMethod {
    
    USE_EMPTY_PACKET,NONE, REPLAY_LAST_SENT_PACKET, USE_SPECIFIED_PAYLOAD, USE_DUMMY_RTP_PAYLOAD;
    
    private static final String USE_EMPTY_PACKET_NAME = "USE-EMPTY-PACKET";
    
    private static final String NONE_NAME = "NONE";
    
    private static final String REPLAY_LAST_SENT_PACKET_NAME = "REPLAY-LAST-SENT-PACKET";
    
    private static final String USE_SPECIFIED_PAYLOAD_NAME = "USE-SPECIFIED-PAYLOAD";
    
    private static final String USE_DUMMY_RTP_PAYLOAD_NAME ="USE-DUMMY-RTP-PAYLOAD";
    
    
    @Override 
    public String toString() {
        if ( this.equals(USE_EMPTY_PACKET)) {
            return USE_EMPTY_PACKET_NAME;
        } else if ( this.equals(NONE)) {
            return NONE_NAME;
        } else if ( this.equals(REPLAY_LAST_SENT_PACKET)) {
            return REPLAY_LAST_SENT_PACKET_NAME;
        } else if ( this.equals(USE_SPECIFIED_PAYLOAD)) {
            return USE_SPECIFIED_PAYLOAD_NAME;
        } else if ( this.equals(USE_DUMMY_RTP_PAYLOAD)) {
            return USE_DUMMY_RTP_PAYLOAD_NAME;
        } else {
            throw new SymmitronException("Bad value") ;
        }
    }
    
    
    public static KeepaliveMethod valueOfString(String methodName) {
        if ( methodName.equals(NONE_NAME)) {
            return NONE;
        } else if ( methodName.equals(REPLAY_LAST_SENT_PACKET_NAME)) {
            return REPLAY_LAST_SENT_PACKET;
        } else if ( methodName.equals(USE_EMPTY_PACKET_NAME)) {
            return USE_EMPTY_PACKET;
        } else if ( methodName.equals(USE_SPECIFIED_PAYLOAD_NAME)) {
            return USE_SPECIFIED_PAYLOAD;
        } else if ( methodName.equals(USE_DUMMY_RTP_PAYLOAD_NAME)) {
            return USE_DUMMY_RTP_PAYLOAD;
        
        } else {
            throw new IllegalArgumentException("Bad value " + methodName);
        }
    }
}
