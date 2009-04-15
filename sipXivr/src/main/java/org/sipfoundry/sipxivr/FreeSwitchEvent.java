/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.HashMap;
import java.util.Vector;

/**
 * A FreeSwitch event is the group of lines that FreeSwitch sends in response to a command or to
 * an asynchronous occurrence. The event may contain a "content-type" header, and if so there is
 * also a String of content (based on the "content-length" header).
 * 
 */
public class FreeSwitchEvent {
    private Vector<String> m_response;
    private String m_content;
    private Exception m_exception;
    private HashMap<String, String> m_headers;
    private HashMap<String, String> m_eventContent;
    private String m_contentType = "(unknown)";
    private String m_eventName = "(unknown)";

    /**
     * Given all the pieces parts (plus some error that occurred), build the event
     * 
     * @param response
     * @param content
     * @param e
     */
    public FreeSwitchEvent(Vector<String> response, String content, Exception e) {
        setResponse(response);
        setContent(content);
        this.m_exception = e;
    }

    /**
     * Given all the pieces parts, build the event
     * 
     * @param response
     * @param content
     */
    public FreeSwitchEvent(Vector<String> response, String content) {
        setResponse(response);
        setContent(content);
    }

    void setResponse(Vector<String> response) {
        this.m_response = response;
    }

    public Vector<String> getResponse() {
        return m_response;
    }

    void setContent(String content) {
        this.m_content = content;
    }

    public String getContent() {
        return m_content;
    }

    public Exception getException() {
        return m_exception;
    }

    public boolean isEmpty() {
        return m_response.size() == 0;
    }

    /**
     * Parse the headers into key/value pairs.
     * 
     * Header names (Keys) are case insensitive, and thus forced to lower-case. Values are URL
     * unescaped.
     * 
     * @param heads A bunch of strings that make up the headers.
     * @return A HashMap of header name values.
     */
    public static HashMap<String, String> parseHeaders(Vector<String> heads) {
        HashMap<String, String> headers = new HashMap<String, String>();
        for (String header : heads) {
            String name = null;
            String value = null;
            int colon = header.indexOf(":");
            if (colon > 0) {
                // A header: value line
                name = header.substring(0, colon).toLowerCase();
                String valueEscaped = header.substring(colon + 1).trim();
                // Unescape URL escaped values
                try {
                    value = URLDecoder.decode(valueEscaped, "UTF-8");
                } catch (UnsupportedEncodingException e) {
                    // Now what?
                    value = valueEscaped;
                }
            } else {
                // A naked header line
                name = header.toLowerCase();
                value = "";
            }
            headers.put(name, value);
        }
        return headers;
    }

    /**
     * Given a header name, find the corresponding value.
     * 
     * @param name
     * @return null if not found, "" if no value, or the value.
     */
    public String getHeader(String name) {
        // Parse the headers if it hasn't already been done
        if (m_headers == null) {
            m_headers = parseHeaders(getResponse());
        }
        return m_headers.get(name.toLowerCase());
    }

    /**
     * Given a header name, find the corresponding value. Don't return null if not found, return
     * onNull instead.
     * 
     * @param name
     * @param onNull The value to return if not found
     * @return onNull if not found, "" if no value, or the value.
     */
    public String getHeader(String name, String onNull) {
        String header = getHeader(name);
        return header == null ? onNull : header;
    }

    /**
     * Take the content part and split it into lines.
     * 
     * @return
     */
    public Vector<String> parseEventContent() {
        Vector<String> contentLines = new Vector<String>();
        BufferedReader in = new BufferedReader(new StringReader(m_content));
        for (;;) {
            String line = null;
            try {
                line = in.readLine();
            } catch (IOException e) {
                // Nothing to do, no where to go home...
            }
            if (line == null || line.equals("")) {
                break;
            }
            contentLines.add(line);
        }
        return contentLines;
    }

    /**
     * Get the value of the Content-Type header if any. "(None)" if none.
     * 
     * @return
     */
    public String getContentType() {
        m_contentType = getHeader("Content-Type", "(None)");
        return m_contentType;
    }

    /**
     * Events of Content-type "text/event-plain" are name value/pairs. Parse them return the value
     * for the given name.
     * 
     * @param name
     * @return null if not found, else the value.
     */
    public String getEventValue(String name) {
        if (!getContentType().contentEquals("text/event-plain")) {
            return null;
        }
        if (m_eventContent == null) {
            m_eventContent = parseHeaders(parseEventContent());
            m_eventName = m_eventContent.get("event-name");
            if (m_eventName == null) {
                m_eventName = "(Unknown)";
            }
        }
        return m_eventContent.get(name.toLowerCase());
    }

    /**
     * Same as above, but return onNull if the value is is null.
     * 
     * @param name
     * @param onNull
     * @return
     */
    public String getEventValue(String name, String onNull) {
        String value = getEventValue(name);
        return value == null ? onNull : value;
    }

    /**
     * For output purposes, use the determined eventName as the "String" value of an event.
     */
    @Override
    public String toString() {
        return m_eventName;
    }

}
