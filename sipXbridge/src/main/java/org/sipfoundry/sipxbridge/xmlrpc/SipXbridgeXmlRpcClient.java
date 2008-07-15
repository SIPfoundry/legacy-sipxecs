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

import java.net.URL;
import java.util.Map;

import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;

/**
 * 
 * Client to invoke methods on the SipxBridgeXmlRpcServer.
 * 
 */
public class SipXbridgeXmlRpcClient {

    private XmlRpcClient client;

    public SipXbridgeXmlRpcClient(String serverAddress, int port) {
        try {
            XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
            config.setServerURL(new URL("http://" + serverAddress + ":" + port));
            this.client = new XmlRpcClient();
            this.client.setConfig(config);
        } catch (Exception ex) {
            throw new SipXbridgeClientException(ex);
        }
    }

   

    public RegistrationRecord[] getRegistrationRecords() {
        Map retval = null;
        try {
            retval = (Map) client.execute(SipXbridgeXmlRpcServer.SERVER + "."
                    + "getRegistrationStatus", (Object[]) null);
        } catch (Exception ex) {
            ex.printStackTrace();
            throw new SipXbridgeClientException(ex);
        }
        if (retval.get(SipXbridgeXmlRpcServer.STATUS_CODE).equals(SipXbridgeXmlRpcServer.ERROR)) {
            throw new SipXbridgeClientException("Error in processing request "
                    + retval.get(SipXbridgeXmlRpcServer.ERROR_INFO));
        }
        Object[] registrations =  (Object[]) retval.get(SipXbridgeXmlRpcServer.REGISTRATION_RECORDS);
        if ( registrations == null ) {
            return null;
        }
        
        
        RegistrationRecord[] registrationRecords = new RegistrationRecord[registrations.length];
        int i = 0;
        for ( Object reg : registrations ) {
            registrationRecords[i++] = RegistrationRecord.create((Map)reg);
        }
        
        return registrationRecords;
        
    }

    
    
    
    public void start() {
        Map retval = null;
        try {
            retval = (Map) client.execute(SipXbridgeXmlRpcServer.SERVER + "."
                    + "start", (Object[]) null);
        } catch (Exception ex) {
            throw new SipXbridgeClientException(ex);
        }
        if (retval.get(SipXbridgeXmlRpcServer.STATUS_CODE).equals(SipXbridgeXmlRpcServer.ERROR)) {
            throw new SipXbridgeClientException("Error in processing request "
                    + retval.get(SipXbridgeXmlRpcServer.ERROR_INFO));
        }
    }
    
    public void stop() {
        Map retval = null;
        try {
            retval = (Map) client.execute(SipXbridgeXmlRpcServer.SERVER + "."
                    + "stop", (Object[]) null);
        } catch (Exception ex) {
            throw new SipXbridgeClientException(ex);
        }
        if (retval.get(SipXbridgeXmlRpcServer.STATUS_CODE).equals(SipXbridgeXmlRpcServer.ERROR)) {
            throw new SipXbridgeClientException("Error in processing request "
                    + retval.get(SipXbridgeXmlRpcServer.ERROR_INFO));
        }
    }

}
