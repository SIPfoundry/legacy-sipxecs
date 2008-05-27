/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.sip;

import java.text.ParseException;

import javax.sip.SipException;
import javax.sip.message.Request;

class NotifyMessage extends JainSipMessage {

    private String m_addrSpec;
    private String m_eventType;

    public NotifyMessage(SipStackBean helper, String addrSpec, String eventType, String contentType, byte[] payload) {
        super(helper, contentType, payload);
        m_addrSpec = addrSpec;
        m_eventType = eventType;
    }

    public NotifyMessage(SipStackBean helper, String addrSpec, String eventType) {
        super(helper);
        m_addrSpec = addrSpec;
        m_eventType = eventType;
    }

    public void createAndSend() {
        try {
            Request request = createRequest(Request.NOTIFY, m_addrSpec);
            getHelper().addEventHeader(request, m_eventType);

            getSipProvider().sendRequest(request);
        } catch (ParseException e) {
            throw new SipxSipException(e);
        } catch (SipException e) {
            throw new SipxSipException(e);
        }
    }
}
