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

import javax.servlet.ServletException;

/**
 * 
 * The XML RPC interface that can be invoked by external clients
 * to interact with sipxbridge.
 * 
 */
public interface SipXbridgeXmlRpcServer {
    
    public static final String SERVER = "SipxBridge";
    
    /**
     * Returns the registration status of each account that 
     * requires registration.
     * @throws ServletException
     */
    public Map<String, String> getRegistrationStatus() throws ServletException;
    
    
    /**
     * Returns the number of ongoing calls.
     * @throws ServletException
     * 
     */
    public Integer getCallCount() throws ServletException;
    
    /**
     * Start the bridge.
     * @throws ServletException
     */
    public Boolean start() throws ServletException;
    
    /**
     * Stop the bridge.
     * @throws ServletException
     */
    public Boolean stop() throws ServletException;
    
    /**
     * Exit the bridge.
     * @throws ServletException
     */
    public Boolean exit() throws ServletException;
  

}
