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
import java.util.Set;

import org.apache.log4j.Logger;
import org.apache.xmlrpc.client.XmlRpcClient;
import org.apache.xmlrpc.client.XmlRpcClientConfigImpl;

/**
 * 
 * Client to invoke methods on the SipxBridgeXmlRpcServer.
 * 
 */
@SuppressWarnings("unchecked")
public class SipXbridgeXmlRpcClient {
	private static Logger logger = Logger.getLogger(SipXbridgeXmlRpcClient.class);

    private XmlRpcClient client;

    public SipXbridgeXmlRpcClient(String serverAddress, int port, boolean isSecure) {
        try {
            XmlRpcClientConfigImpl config = new XmlRpcClientConfigImpl();
        	config.setEnabledForExceptions(true);
			config.setEnabledForExtensions(true);
			String url = (isSecure? "https" : "http") + "://" + serverAddress + ":" + port;
			if ( logger.isDebugEnabled() ) logger.debug("SipXbridgeXmlRpcClient: url =  " + url);
            config.setServerURL(new URL(url));
            this.client = new XmlRpcClient();
            this.client.setConfig(config);
        } catch (Exception ex) {
            throw new SipXbridgeClientException(ex);
        }
    }

   
    /**
     * Gets an array of Registration records - one record for each account
     * that requires registration.
     * 
     * @return an array of registration records.
     */

    public RegistrationRecord[] getRegistrationRecords() {
        Map retval = null;
        try {
            retval = (Map) client.execute(SipXbridgeXmlRpcServer.SERVER + "."
                    + "getRegistrationStatus", (Object[]) null);
        } catch (Exception ex) {
            ex.printStackTrace();
            throw new SipXbridgeClientException(ex);
        }

        if ( retval == null ) {
            return null;
        }

        RegistrationRecord[] registrationRecords = new RegistrationRecord[retval.size()];
        int i = 0;
        Set keys = retval.keySet();
        for ( Object key : keys) {
            registrationRecords[i++] = new RegistrationRecord((String) key, (String) retval.get(key));
        }

        return registrationRecords;
    }
    
    
    /**
     * Get a count of the number of ongoing calls.
     * 
     * @return the number of ongoing calls.
     */
    public int getCallCount() {
        Integer retval;
        try {
             retval = (Integer) client.execute(SipXbridgeXmlRpcServer.SERVER + "."
                    + "getCallCount", (Object[]) null);
        } catch (Exception ex) {
            ex.printStackTrace();
            throw new SipXbridgeClientException(ex);
        }

        return retval;
    }

    public void start() {
        try {
            client.execute(SipXbridgeXmlRpcServer.SERVER + "."
                    + "start", (Object[]) null);
        } catch (Exception ex) {
            throw new SipXbridgeClientException(ex);
        }
    }

	public void stop() {
        try {
            client.execute(SipXbridgeXmlRpcServer.SERVER + "."
                    + "stop", (Object[]) null);
        } catch (Exception ex) {
            throw new SipXbridgeClientException(ex);
        }
    }
    
    public void exit() {
        try {
            client.execute(SipXbridgeXmlRpcServer.SERVER + "."
                    + "exit", (Object[]) null);
        } catch (Exception ex) {
        	logger.error("Exception in exit() ", ex);
            throw new SipXbridgeClientException(ex);
        }
    }

}
