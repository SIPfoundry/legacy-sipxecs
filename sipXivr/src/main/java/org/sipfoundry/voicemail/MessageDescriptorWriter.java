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

import java.io.IOException;
import java.io.Writer;

import org.apache.log4j.Logger;
import org.dom4j.Document;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.dom4j.QName;
import org.dom4j.io.OutputFormat;
import org.dom4j.io.XMLWriter;


public class MessageDescriptorWriter extends XmlWriterImpl<MessageDescriptor> {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    static final DocumentFactory FACTORY = DocumentFactory.getInstance();
    private MessageDescriptor m_messageDescriptor;
    
    @Override
    public void writeObject(MessageDescriptor messageDescriptor, Writer output) {
        m_messageDescriptor = messageDescriptor ;
        OutputFormat format = new OutputFormat();
        format.setNewlines(true);
        format.setIndent(true);
        XMLWriter xmlWriter = new XMLWriter(output, format);
        try {
            xmlWriter.write(getDocument());
        } catch (IOException e) {
            LOG.error("Error writing MessageDescriptor to "+output.toString(), e);
            throw new RuntimeException(e);
        }
    }

    private Document getDocument() {
        Document document = FACTORY.createDocument();
        QName prefsQ = FACTORY.createQName("messagedescriptor");
        Element prefsEl = document.addElement(prefsQ);
        addAndSet(prefsEl, "id", m_messageDescriptor.getId());
        addAndSet(prefsEl, "from", m_messageDescriptor.getFromUri());
        addAndSet(prefsEl, "durationsecs", m_messageDescriptor.getDurationSecs());
        addAndSet(prefsEl, "timestamp", m_messageDescriptor.getTimestampString());
        addAndSet(prefsEl, "subject", m_messageDescriptor.getSubject());
        addAndSet(prefsEl, "priority", m_messageDescriptor.getPriority().getId());
        if(m_messageDescriptor.getOtherRecipients() != null) {
            for(String otherRecipient : m_messageDescriptor.getOtherRecipients()) {
                addAndSet(prefsEl, "other_recipient", otherRecipient); 
            }
        }
        return document;
    }
    
    private void addAndSet(Element el, String name, String text) {
        Element subel = el.addElement(name);
        if (text != null) {
            subel.setText(text) ;
        }
    }
}
