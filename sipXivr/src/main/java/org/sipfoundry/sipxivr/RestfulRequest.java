/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxivr;

import java.io.BufferedReader;
import java.io.InputStreamReader;
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
    String m_content;
    String m_contentType;
    String m_response;
    int m_responseCode;
    Exception m_exception;

    public RestfulRequest(String urlString) {
        m_urlString = urlString;
    }

    public RestfulRequest(String urlString, String user, String password) {
        m_urlString = urlString;
        if (user != null && password != null) {
            m_digest = "Basic " + new String(Base64.encodeBase64((user+":"+password).getBytes()));
        }
    }

    /**
     * Initialize the connection parameters then connect via http/https
     * @param value Added to m_urlString to complete the URL.  Redacted from logs.
     * @return an HttpURLConnection ready to use with {@link request}
     * @throws Exception
     */
    public HttpURLConnection getConnection(String value) throws Exception {
        String urlString = m_urlString;
        if (value != null) {
            if (!urlString.endsWith("/")) {
                urlString += "/";
            }
            urlString += value;
        }

        URL url = new URL(urlString);

        // URL connection channel.
        HttpURLConnection urlConn = (HttpURLConnection) url.openConnection();

        // Set the Authorization header to the encoded user/passcode
        // combination
        if (m_digest != null) {
            urlConn.setRequestProperty("Authorization", m_digest);
        }

        return urlConn;
    }


    /**
     *  Perform the method on the URLConnection
     *
     * @param method  GET, POST, PUT, DELETE, etc.
     * @param urlConn a HttpURLConnection from {@link getConnection}
     * @return true on valid response, false on failure
     * @throws Exception
     */
    public boolean request(String method, HttpURLConnection urlConn) throws Exception {
        if (urlConn == null) {
            throw new Exception("Invalid connection reference");
        }

        if (m_justTesting) {
            // Don't do anything if we are just in a test situation
            m_content = m_testingResult;
            return true;
        }

        try {
            // Let the RTS know that we want to do both input and output.
            urlConn.setDoOutput(true);
            urlConn.setDoInput(true);

            // No caching, we want the real thing.
            urlConn.setUseCaches(false);
            urlConn.setRequestMethod(method);

            // Connect!
            urlConn.connect();
            
            // Find out what happened
            m_responseCode = urlConn.getResponseCode();
            m_response = urlConn.getResponseMessage();

            LOG.info(String.format("RestfulRequest::request %s to %s response: %d", method, m_urlString,
                    m_responseCode));

            if (m_responseCode < 200 || m_responseCode >= 300) {
                // Responses 200-299 are Okay, all other are suspect
                return false;
            }

            return true;
        }

        catch (Exception ex) {
            m_exception = ex;
            LOG.error(String.format("RestfulRequest::request %s to '%s' trouble '%s'", method, urlConn.getURL().toString(),
                    urlConn.getResponseMessage()), ex);
        }
        return false;
    }

    /**
     * Convenience method to send a request to a URL
     * 
     * @param method an HTTP method, PUT, DELETE, POST, GET, etc. 
     *        where no body is provided.  
     *        Any response (assumed to be text) is in m_content;
     * @throws Exception 
     */
    boolean send(String method, String value) throws Exception{
        HttpURLConnection urlConn = getConnection(value);
        boolean result = request(method, urlConn);
        if (urlConn.getContentLength() > 0) {
            m_contentType = urlConn.getContentType();
            BufferedReader br = new BufferedReader(new InputStreamReader(urlConn.getInputStream()));
            String line ;
            m_content = "";
            for(line = br.readLine(); line != null;) {
                m_content.concat(line);
                m_content.concat("\n");
            }
        }
        urlConn.disconnect();
        return result;
    }
    

    /**
     * Convenience method to PUT to a URL (no body)
     * 
     * @param value A string added to the end of the url.  Redacted from logs.
     * @throws Exception 
     */
    public boolean put(String value) throws Exception{
        return send("PUT", value);
    }

    /**
     * Convenience method to POST to a URL (no body)
     * 
     * @param value A string added to the end of the url.  Redacted from logs.
     * @throws Exception 
     */
    public boolean post(String value) throws Exception{
        return send("POST", value);
    }

    /**
     * Convenience method to DELETE to a URL
     * 
     * @throws Exception 
     */
    public boolean delete() throws Exception{
        return send("DELETE", null);
    }

    /**
     * Convenience method to GET to a URL
     * 
     * @throws Exception 
     */
    public boolean get() throws Exception{
        return send("GET", null);
    }

    
    /**
     * method to GET content from a URL with a reusable connection
     * No content is read from the connection, that's up to the caller. 
     * 
     * @param urlConn a HttpURLConnection from {@link getConnection}
     * @throws Exception 
     */
    public boolean get(HttpURLConnection urlConn) throws Exception{
        return request("GET", urlConn);
    }
    
    public int getResponseCode() {
        return m_responseCode ;
    }
    
    public String getResponse() {
        return m_response;
    }
    
    public String getResult() {
        return m_content;
    }
    
    public String getContentType() {
        return m_contentType;
    }
    
    public String getContent() {
        return m_content;
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
