/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import javax.sip.SipFactory;
import javax.sip.address.AddressFactory;
import javax.sip.header.HeaderFactory;
import javax.sip.message.MessageFactory;

/**
 * A static class that has pointers to all the factories.
 *
 * @author M. Ranganathan
 *
 */
public class SipFactories {
    public static final HeaderFactory headerFactory;
    public static final MessageFactory messageFactory;
    public static final AddressFactory addressFactory;
    public static final SipFactory sipFactory;

    static {
        try {
            sipFactory = SipFactory.getInstance();
            sipFactory.setPathName("gov.nist");
            headerFactory = sipFactory.createHeaderFactory();
            messageFactory = sipFactory.createMessageFactory();
            addressFactory = sipFactory.createAddressFactory();
        } catch (Exception ex) {
            throw new RuntimeException("Error initializing factories", ex);
        }

    }
}
