/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.voicemail;

import java.util.List;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class MessageDescriptorReader extends XmlReaderImpl<MessageDescriptor> {
    
    private Node m_root;
    
    @SuppressWarnings("unchecked")
    @Override
    public MessageDescriptor readObject(Document doc) {
        MessageDescriptor md = new MessageDescriptor();
        m_root = doc.getRootElement();
        md.setId(valueOf("id"));
        md.setFromUri(valueOf("from"));
        md.setDurationSecs(valueOf("durationsecs"));
        md.setTimestamp(valueOf("timestamp"));
        md.setSubject(valueOf("subject"));
        md.setPriority((Priority.valueOfById(valueOf("priority"))));
        
        List<Element> recipients = m_root.selectNodes("otherrecipient");
        for (Element recipient : recipients) {
            md.addOtherRecipient(recipient.getTextTrim());
        }
        
        return md;
    }

    /**
     * Lookup name at root level. 
     * @param name
     * @return value of name (null if missing or empty)
     */
    private String valueOf(String name) {
        String value = m_root.valueOf(name);
        if (value != null && value.length() == 0) {
            value = null;
        }
        return value;
    }
}
