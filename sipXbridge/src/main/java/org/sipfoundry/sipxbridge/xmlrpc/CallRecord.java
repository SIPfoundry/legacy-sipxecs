/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.sipxbridge.xmlrpc;

import java.util.Date;
import java.util.HashMap;
import java.util.Map;

/**
 * 
 * Call record that can be queried from sipxconfig. This 
 * returns a record of an ongoing call.
 * 
 */
public class CallRecord {
    private String callId;
    private static final String CALL_ID = "callId";
    private String fromAddress;
    private static final String FROM_ADDRESS = "fromAddress";
    private String toAddress;
    private static final String TO_ADDRESS = "toAddress";
    private String requestURI;
    private static final String REQUEST_URI = "requestUri";
    private String startTime;
    private static final String START_TIME = "startTime";
    
    
    public Map<String,String> getMap() {
        
        Map<String,String> retval = new HashMap<String,String>();
        retval.put(CALL_ID, callId);
        retval.put(FROM_ADDRESS, fromAddress);
        retval.put(TO_ADDRESS, toAddress );
        retval.put(START_TIME, startTime);
        retval.put(REQUEST_URI,requestURI);
        return retval;
    }
    
    public static CallRecord create(Map<String,Object> map) {
        CallRecord callRecord = new CallRecord();
        callRecord.fromAddress = (String) map.get(FROM_ADDRESS);
        callRecord.toAddress = (String) map.get(TO_ADDRESS);
        callRecord.requestURI = (String)map.get(REQUEST_URI);
        callRecord.startTime = (String) map.get(START_TIME);
        return callRecord;
        
    }
    
    public CallRecord() {
        this.startTime  = new Date().toString();
    }
    /**
     * @return the callRecordId
     */
    public String getCallId() {
        return callId;
    }
    
    /**
     * set the call Id.
     */
    public void setCallId(String callId) {
        this.callId = callId;
    }
    /**
     * @param fromAddress the fromAddress to set
     */
    public void setFromAddress(String fromAddress) {
        this.fromAddress = fromAddress;
    }
    /**
     * @return the fromAddress
     */
    public String getFromAddress() {
        return fromAddress;
    }
    /**
     * @param toAddress the toAddress to set
     */
    public void setToAddress(String toAddress) {
        this.toAddress = toAddress;
    }
    /**
     * @return the toAddress
     */
    public String getToAddress() {
        return toAddress;
    }
    /**
     * @param startTime the startTime to set
     */
    public void setStartTime(String startTime) {
        this.startTime = startTime;
    }
    /**
     * @return the startTime
     */
    public String getStartTime() {
        return startTime;
    }
    /**
     * @param requestURI the requestURI to set
     */
    public void setRequestURI(String requestURI) {
        this.requestURI = requestURI;
    }
    /**
     * @return the requestURI
     */
    public String getRequestURI() {
        return requestURI;
    }
    

}
