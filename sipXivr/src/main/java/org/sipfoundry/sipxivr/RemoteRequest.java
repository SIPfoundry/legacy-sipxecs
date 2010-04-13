/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxivr;

import java.io.DataOutputStream;
import java.net.HttpURLConnection;
import java.net.URL;

import org.apache.log4j.Logger;


public class RemoteRequest {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    boolean m_justTesting;
    String m_testingResult;
    
    URL m_url;
    String m_contentType;
    String m_content;
    
    String m_result;
    String m_response;
    Exception m_exception;
    
    public RemoteRequest(URL url, String contentType, String content) {
        m_url = url;
        m_contentType = contentType;
        m_content = content;
    }
    
    /**
     * Connect via http/https, POST content
     * @throws Exception 
     */
    public boolean http() throws Exception {
        if (m_justTesting) {
            // Don't do anything if we are just in a test situation
            m_result = m_testingResult;
            return true;
        }

        HttpURLConnection urlConn = null;
        try {
            DataOutputStream printout;

            // URL connection channel.
            urlConn = (HttpURLConnection) m_url.openConnection();

            // Let the run-time system (RTS) know that we want input.
            urlConn.setDoInput(true);

            // Let the RTS know that we want to do output.
            urlConn.setDoOutput(true);

            // No caching, we want the real thing.
            urlConn.setUseCaches(false);

            // Specify the content type.
            urlConn.setRequestProperty("Content-Type", m_contentType);


            LOG.debug(String.format("RemoteRequest::httpRequest to %s posting %s", m_url.toExternalForm(), m_content));
            printout = new DataOutputStream(urlConn.getOutputStream());

            printout.writeBytes(m_content);
            printout.flush();
            printout.close();
            
            m_response = urlConn.getResponseMessage();
 
/*
            DataInputStream input;
            input = new DataInputStream(urlConn.getInputStream());
            m_result = input.readUTF();
            input.close();
*/            
            return true;
        }

        catch (Exception ex) {
            m_exception = ex;
            LOG.error(String.format("RemoteRequest::httpRequest to '%s' trouble '%s'", 
                    m_url.toExternalForm(), m_response), ex);
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
