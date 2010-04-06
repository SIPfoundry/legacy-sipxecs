/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.fswitchtester;
import gov.nist.javax.sip.SipStackImpl;
import gov.nist.javax.sip.message.MessageFactoryImpl;

import java.io.File;
import java.util.LinkedList;
import java.util.List;
import java.util.Properties;

import javax.sip.SipException;
import javax.sip.SipFactory;
import javax.sip.SipStack;
import javax.sip.address.AddressFactory;
import javax.sip.header.HeaderFactory;
import javax.sip.header.UserAgentHeader;
import javax.sip.message.MessageFactory;

/**
 * Sip Protocol objects.
 * 
 * @author M. Ranganathan
 * 
 */
public class ProtocolObjects {
    public static final HeaderFactory headerFactory;
    public static final MessageFactory messageFactory;
    public static final AddressFactory addressFactory;
    public static final SipFactory sipFactory;
    public static final SipStack sipStack;

    static {
        try {
            sipFactory = SipFactory.getInstance();
            sipFactory.setPathName("gov.nist");
            headerFactory = sipFactory.createHeaderFactory();
            messageFactory = sipFactory.createMessageFactory();
            

            UserAgentHeader userAgentHeader = SipUtilities.createUserAgentHeader();
            ((MessageFactoryImpl) messageFactory)
                    .setCommonUserAgentHeader(userAgentHeader);
            addressFactory = sipFactory.createAddressFactory();
            Properties stackProperties = new Properties();
            stackProperties.setProperty("javax.sip.STACK_NAME",
                    "org.sipfoundry.sipxbridge");
            stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL",
                    FreeSwitchTester.getLogLevel());
            stackProperties.setProperty(
                    "gov.nist.javax.sip.LOG_MESSAGE_CONTENT", "true");
          
            stackProperties.setProperty("gov.nist.javax.sip.LOG_FACTORY",
                    SipFoundryLogRecordFactory.class.getName());
            
            
           
            sipStack = ProtocolObjects.sipFactory
                    .createSipStack(stackProperties);
            ((SipStackImpl) sipStack)
                    .setAddressResolver(new ProxyAddressResolver());
          
            ((SipStackImpl) sipStack).addLogAppender(new SipFoundryAppender(
                    new SipFoundryLayout(), FreeSwitchTester.getLogFile()));

        } catch (Exception ex) {
            throw new RuntimeException("Error loading factories", ex);
        }

    }

    public static void start() throws SipException {
        sipStack.start();
    }

    public static void stop() {
        sipStack.stop();

    }

}


