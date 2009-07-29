/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxivr;

import java.util.List;

import org.apache.commons.codec.binary.Base64;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Node;
import org.sipfoundry.voicemail.XmlReaderImpl;

public class MailboxPreferencesReader extends XmlReaderImpl<MailboxPreferences> {
    
    @SuppressWarnings("unchecked")
    @Override
    public MailboxPreferences readObject(Document doc) {
        MailboxPreferences prefs = new MailboxPreferences();
        Node root = doc.getRootElement();
        String greetingId = root.valueOf("activegreeting");
        MailboxPreferences.GreetingType greeting = MailboxPreferences.GreetingType.valueOfById(greetingId); 
        prefs.setActiveGreeting(greeting);
        
        List<Element> imapconfig = root.selectNodes("imapserver");
        prefs.setIMAPServer(getIMAPServer(imapconfig));  
        prefs.setIMAPPortNum(getIMAPPortNum(imapconfig));  
        prefs.setUseTLS(getUseTLS(imapconfig));
        
        List<Element> contacts = root.selectNodes("notification/contact");
        prefs.setEmailAddress(getEmailAddress(0, contacts));
        prefs.setAlternateEmailAddress(getEmailAddress(1, contacts));
        prefs.setAttachVoicemailToEmail(getAttachVoicemail(0, contacts));
        prefs.setSynchronize(getSynchronize(contacts));
        prefs.setemailPassword(getPassword(contacts));
        
        prefs.setAttachVoicemailToAlternateEmail(getAttachVoicemail(1, contacts));
        return prefs;
    }
    
    private String getEmailAddress(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return null;
        }
        return contacts.get(index).getText();
    }
    
    private boolean getSynchronize(List<Element> contacts) {
        if (contacts.size() == 0) {
            return false;
        }
        String sSynchronize = contacts.get(0).attributeValue("synchronize");
        return "yes".equals(sSynchronize);
    }
    
    private String getPassword(List<Element> contacts) {
        if (contacts.size() == 0) {
            return null;
        }
        
        String pwd = new String(Base64.decodeBase64(contacts.get(0).attributeValue("password").getBytes()));
        return pwd;          
    }
    
    private String getIMAPServer(List<Element> imapconfig) {        
        if(imapconfig.size() == 0) {
            return null;
        }
        
        return imapconfig.get(0).attributeValue("host");
    }
    
    private Integer getIMAPPortNum(List<Element> imapconfig) {    
        if(imapconfig.size() == 0) {
            return 0;
        }
        
        return Integer.parseInt(imapconfig.get(0).attributeValue("port"));
    }
    
    private boolean getUseTLS(List<Element> imapconfig) {
        if(imapconfig.size() == 0) {
            return false;
        }
 
        String sUseTLS = imapconfig.get(0).attributeValue("UseTLS");
        return "yes".equals(sUseTLS);
    }
    
    private boolean getAttachVoicemail(int index, List<Element> contacts) {
        if (contacts.size() <= index) {
            return false;
        }
        String sAttachVm = contacts.get(index).attributeValue("attachments");
        return "yes".equals(sAttachVm);
    }
}
