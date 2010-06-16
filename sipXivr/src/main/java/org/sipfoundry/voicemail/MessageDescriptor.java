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

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.Vector;



/**
 * Holds the MessageDescriptor for a Voicemail message.
 * Maps directly to the xml in MessageDescriptorReader/Writer
 */
public class MessageDescriptor {
    private static final String DATE_FORMAT="EEE, d-MMM-yyyy hh:mm:ss aaa z";
    
    private String m_id; // The id is an almost URI (user@domain) of the recipient
    private String m_fromUri;
    private String m_durationSecs;
    private String m_timestamp;
    private String m_subject;
    private Priority m_priority = Priority.NORMAL;
    private Vector<String> m_otherRecipients;
         
    public enum Priority {
        NORMAL("normal"),
        URGENT("urgent");
        
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
   
    public long getDurationSecsLong() {
        return Long.parseLong(m_durationSecs);
    }

    public void setDurationSecs(long durationSecs) {
        m_durationSecs = Long.toString(durationSecs);
    }

    public Date getTimeStampDate() {

        SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT, Locale.ENGLISH);
        try {
            return dateFormat.parse(m_timestamp);
        } catch (ParseException e) {
            // hmmm .. lets try the default locale (for backward compatibility)
            dateFormat = new SimpleDateFormat(DATE_FORMAT);
            try {
                return dateFormat.parse(m_timestamp);
            } catch (ParseException e1) {
                return null;
            }
        }
    }
    
    public String getTimestampString() {
        return m_timestamp;
    }
    
    public void setTimestamp(String timestamp) {
        m_timestamp = timestamp;
    }
    
    public void setTimestamp(long timestamp) {
        // Use RFC-2822 format
        SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT, Locale.ENGLISH);
        m_timestamp = dateFormat.format(timestamp); 
    }
    
    public String getSubject() {
        return m_subject;
    }
    
    public void addOtherRecipient(String recipient) {
        if(m_otherRecipients == null) {
            m_otherRecipients = new Vector<String>();
        }
        m_otherRecipients.add(recipient);
    }
    
    public void removeOtherRecipient(String recipient) {
        if(m_otherRecipients != null) {
            m_otherRecipients.remove(recipient);
        }
    }
    
    public Vector<String> getOtherRecipients() {
        return m_otherRecipients;
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
