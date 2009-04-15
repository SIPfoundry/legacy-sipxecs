/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */

package org.sipfoundry.sipxbridge.xmlrpc;

import java.util.Map;

/**
 * 
 * The XML RPC interface that can be invoked by external clients
 * to interact with sipxbridge.
 * 
 */
public interface SipXbridgeXmlRpcServer {
    
    public static final String SERVER = "SipxBridge";
    /**
     * The status code OK or ERROR.
     */
    public static final String STATUS_CODE = "status-code";
    
    /**
     * Status code of OK if no error was encountered.
     */
    public static final String OK = "OK";
    
    
    public String ERROR = "ERROR";
  
    /**
     * A standard error code -- see definitions below.
     */
    public static final String ERROR_CODE = "faultCode";

    /**
     * Detailed error information.
     */
    public static final String ERROR_INFO = "faultString";

    /**
     * Data element for call records.
     */
    public static final String CALL_RECORDS = "callRecords";
    
    /**
     * Data element for registration records
     */
    public static final String REGISTRATION_RECORDS = "registrationRecords";
    
    /**
     * Data element for number of ongoing calls.
     */
    public static final String CALL_COUNT = "callCount";
    
    
    
    /**
     * Returns the registration status of each account that 
     * requires registration.
     */
    public Map<String,Object> getRegistrationStatus();
    
    
    /**
     * Returns the number of ongoing calls.
     * 
     */
    public Map<String,Object> getCallCount();
    
    /**
     * Start the bridge.
     */
    public Map<String,Object> start();
    
    /**
     * Stop the bridge.
     */
    public Map<String,Object> stop();
    
    /**
     * Exit the bridge.
     */
    public Map<String,Object> exit();
  

}
