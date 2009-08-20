/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.SipStackImpl;
import gov.nist.javax.sip.header.ViaList;
import gov.nist.javax.sip.message.MessageFactoryImpl;

import java.util.Properties;

import javax.sip.SipException;
import javax.sip.SipFactory;
import javax.sip.SipStack;
import javax.sip.address.AddressFactory;
import javax.sip.header.HeaderFactory;
import javax.sip.header.ServerHeader;
import javax.sip.header.UserAgentHeader;
import javax.sip.message.MessageFactory;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.log4j.SipFoundryLogRecordFactory;

/**
 * Sip Protocol objects.
 *
 * @author M. Ranganathan
 *
 */
public class ProtocolObjects {
    private static final Logger logger = Logger.getLogger(ProtocolObjects.class);
    public static final HeaderFactory headerFactory;
    public static final MessageFactory messageFactory;
    public static final AddressFactory addressFactory;
    public static final SipFactory sipFactory;
    private static  SipStack sipStack;

    static {
        try {
            sipFactory = SipFactory.getInstance();
            sipFactory.setPathName("gov.nist");
            headerFactory = sipFactory.createHeaderFactory();
            messageFactory = sipFactory.createMessageFactory();

            UserAgentHeader userAgentHeader = SipUtilities.createUserAgentHeader();
            ((MessageFactoryImpl) messageFactory).setDefaultUserAgentHeader(userAgentHeader);
            ServerHeader serverHeader = SipUtilities.createServerHeader();
            ((MessageFactoryImpl) messageFactory).setDefaultServerHeader(serverHeader);
            addressFactory = sipFactory.createAddressFactory();
        } catch (Exception ex) {
            ex.printStackTrace();
            throw new SipXbridgeException("Error loading factories", ex);
        }
    }

    private static void createSipStack() {
        try {

            Properties stackProperties = new Properties();
            stackProperties.setProperty("javax.sip.STACK_NAME", "org.sipfoundry.sipXbridge");
            if (!Gateway.getLogLevel().equalsIgnoreCase("TRACE")) {
                if (Gateway.getLogLevel().equalsIgnoreCase("DEBUG")) {
                    /*
                     * Debug level turns off stack level debug logging.
                     */
                    stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", Level.INFO
                            .toString());
                    stackProperties.setProperty(
                            "gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "true");
                } else {
                    stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", Gateway
                            .getLogLevel());
                    stackProperties.setProperty(
                            "gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "false");
                }

            } else {
                /*
                 * At TRACE level you get a LOT of logging.
                 */
                stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", Level.TRACE
                        .toString());
            }
            stackProperties.setProperty("gov.nist.javax.sip.REENTRANT_LISTENER", "true");
            stackProperties.setProperty("gov.nist.javax.sip.LOG_MESSAGE_CONTENT", "true");
            stackProperties.setProperty("gov.nist.javax.sip.LOG_FACTORY",
                    SipFoundryLogRecordFactory.class.getName());
            stackProperties.setProperty("javax.sip.ROUTER_PATH",
                    org.sipfoundry.commons.siprouter.ProxyRouter.class.getName());
            stackProperties.setProperty("gov.nist.javax.sip.CACHE_CLIENT_CONNECTIONS", "true");
            stackProperties.setProperty("gov.nist.javax.sip.CACHE_SERVER_CONNECTIONS", "true");
            stackProperties.setProperty("gov.nist.javax.sip.REJECT_STRAY_RESPONSES","true");
            /*
             * Break up the via encoding.
             */
            ViaList.setPrettyEncode(true);
            sipStack = ProtocolObjects.sipFactory.createSipStack(stackProperties);
            ((SipStackImpl) getSipStack()).addLogAppender(Gateway.logAppender);
        } catch (Exception ex) {
            ex.printStackTrace();
            logger.error("Error loading factories ", ex);
            throw new SipXbridgeException("Error loading factories", ex);
        }

    }

    public static void start() throws SipException {
        getSipStack().start();
    }

    public static void stop() {
        getSipStack().stop();

    }

    /**
     * @return the sipStack
     */
    public static SipStack getSipStack() {
        if ( sipStack == null ) {
            createSipStack();
        }
        return sipStack;
    }

}
