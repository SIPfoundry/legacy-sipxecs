/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.voicemail;

import java.text.SimpleDateFormat;



/**
 * Holds the MessageDescriptor for a Voicemail message.
 * Maps directly to the xml in MessageDescriptorReader/Writer
 */
public class MessageDescriptor {
    private String m_id;
    private String m_fromUri;
    private String m_durationSecs;
    private String m_timestamp;
    private String m_subject;
    private Priority m_priority = Priority.NORMAL;
         
    public enum Priority {
        NORMAL("normal") ;
        
        private String m_id;
        
        Priority(String id) {
            m_id = id;
        }
        
        public String getId() {
            return m_id;
        }
        
        public static Priority valueOfById(String id) {
            for (Priority p : Priority.values()) {
                if (p.getId().equals(id)) {
                    return p;
                }
            }
            throw new IllegalArgumentException("id not recognized " + id);
        }
    }

    public String getId() {
        return m_id;
    }
    
    public void setId(String id) {
        m_id = id;
    }
    
    public String getFromUri() {
        return m_fromUri;
    }
    
    public void setFromUri(String uri) {
        m_fromUri = uri;
    }
    
    public String getDurationSecs() {
        return m_durationSecs;
    }
    
    public void setDurationSecs(String durationSecs) {
        m_durationSecs = durationSecs;
    }
    
    public String getTimestamp() {
        return m_timestamp;
    }
    
    public void setTimestamp(String timestamp) {
        m_timestamp = timestamp;
    }
    
    public void setTimestamp(long timestamp) {
        // Use RFC-2822 format
        SimpleDateFormat dateFormat = new SimpleDateFormat("EEE, d-MMM-yyyy hh:mm:ss aaa z");
        m_timestamp = dateFormat.format(timestamp); 
    }
    
    public String getSubject() {
        return m_subject;
    }
    
    public void setSubject(String subject) {
        m_subject = subject;
    }
    
    public Priority getPriority() {
        return m_priority;
    }
    
    public void setPriority(Priority priority) {
        m_priority = priority;
    }
}
