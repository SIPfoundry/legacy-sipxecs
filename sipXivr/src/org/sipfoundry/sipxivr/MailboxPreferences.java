/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxivr;


/**
 * Final output format
 * <pre>
 * &lt;prefs&gt;
 *   &lt;activegreeting&gt;outofoffice&lt;/activegreeting&gt;
 * &lt;/prefs&gt;
 * </pre>
 */
public class MailboxPreferences {
    private ActiveGreeting m_activeGreeting = new ActiveGreeting();
         
    public ActiveGreeting getActiveGreeting() {
//        return m_activeGreeting.getGreetingType();
        return m_activeGreeting;
    }
    
    public void setActiveGreeting(ActiveGreeting activeGreeting) {
        m_activeGreeting = activeGreeting;
    }
}
