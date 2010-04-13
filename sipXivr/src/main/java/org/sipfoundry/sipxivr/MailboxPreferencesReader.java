/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */
package org.sipfoundry.sipxivr;

import org.dom4j.Document;
import org.dom4j.Node;
import org.sipfoundry.voicemail.XmlReaderImpl;

public class MailboxPreferencesReader extends XmlReaderImpl<MailboxPreferences> {
    
    @Override
    public MailboxPreferences readObject(Document doc) {
        MailboxPreferences prefs = new MailboxPreferences();
        Node root = doc.getRootElement();
        String greetingId = root.valueOf("activegreeting");
        GreetingType greeting = GreetingType.valueOfById(greetingId); 
        prefs.getActiveGreeting().setGreetingType(greeting);
        
        return prefs;
    }
}
