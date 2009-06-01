/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxivr;

import java.net.HttpURLConnection;
import java.net.URL;

import org.apache.commons.codec.binary.Base64;
import org.apache.log4j.Logger;


public class RestfulRequest {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    boolean m_justTesting;
    String m_testingResult;
    
    String m_urlString;
    String m_digest;
    
    String m_result;
    String m_response;
    Exception m_exception;
    
    public RestfulRequest(String urlString, String user, String password) {
        m_urlString = urlString;
        if (!m_urlString.endsWith("/")) {
            m_urlString += "/";
        }
        m_digest = "Basic " + new String(Base64.encodeBase64((user+":"+password).getBytes()));
    }
    
    /**
     * Connect via http/https, put content
     * @throws Exception 
     */
    public boolean put(String value) throws Exception {
        if (m_justTesting) {
            // Don't do anything if we are just in a test situation
            m_result = m_testingResult;
            return true;
        }

        HttpURLConnection urlConn = null;
        try {
            URL url = new URL(m_urlString+value);
            // URL connection channel.
            urlConn = (HttpURLConnection) url.openConnection();

            // Set the Authorization header to the encoded user/passcode combination
            urlConn.setRequestProperty ("Authorization", m_digest);
            
            // Let the RTS know that we want to do output.
            urlConn.setDoOutput(true);

            // No caching, we want the real thing.
            urlConn.setUseCaches(false);

            urlConn.setRequestMethod("PUT");

            LOG.info(String.format("RestfulRequest::put to %s/(_REDACTED_)", m_urlString));

            m_response = urlConn.getResponseMessage();
 
            if (urlConn.getResponseCode() < 200 || urlConn.getResponseCode() >=300) {
                // Responses 200-299 are Okay, all other are suspect
                return false;
            }
            
            return true;
        }

        catch (Exception ex) {
            m_exception = ex;
            LOG.error(String.format("RestfulRequest::put to '%s' trouble '%s'", 
                    m_urlString, m_response), ex);
        }
        return false;
    }

    public String getResponse() {
        return m_response;
    }
    
    public String getResult() {
        return m_result;
    }
    
    public Exception getException() {
        return m_exception;
    }
    
    public boolean isJustTesting() {
        return m_justTesting;
    }

    public void setJustTesting(boolean justTesting) {
        m_justTesting = justTesting;
    }
    
    public void setTestingResult(String result) {
        m_testingResult = result;
    }
}
