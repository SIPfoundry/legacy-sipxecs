/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

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
import org.sipfoundry.commons.log4j.ServerLoggerImpl;
import org.sipfoundry.commons.log4j.SipFoundryLogRecordFactory;
import org.sipfoundry.commons.log4j.StackLoggerImpl;


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
            /*
             * At this point we have already added an appender to this logger.
             * Just get the logger for the gateway and give it to the stack.
             */
            Logger logger = Logger.getLogger(Gateway.class.getPackage().getName());
         
            String logLevel = Gateway.getLogLevel();
            if (logLevel.equalsIgnoreCase("DEBUG") || logLevel.equalsIgnoreCase("TRACE")) {
                stackProperties.setProperty("gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "true");
            } else {
                stackProperties.setProperty("gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "false");
            }

            // stack log levels are "off by one": otherwise too much logging at DEBUG level
            if (logLevel.equalsIgnoreCase("TRACE")) {
                logLevel = "DEBUG";
            }
            else if (logLevel.equalsIgnoreCase("DEBUG")) {
                logLevel = "INFO";
            }
            stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", logLevel);

            stackProperties.setProperty("gov.nist.javax.sip.REENTRANT_LISTENER", "true");
            stackProperties.setProperty("gov.nist.javax.sip.LOG_MESSAGE_CONTENT", "true");
            stackProperties.setProperty("gov.nist.javax.sip.LOG_FACTORY",
                    SipFoundryLogRecordFactory.class.getName());
            stackProperties.setProperty("javax.sip.ROUTER_PATH",
                    org.sipfoundry.commons.siprouter.ProxyRouter.class.getName());
            stackProperties.setProperty("gov.nist.javax.sip.TLS_SECURITY_POLICY", SipxTlsSecurityPolicy.class.getName());
            stackProperties.setProperty("gov.nist.javax.sip.CACHE_CLIENT_CONNECTIONS", "true");
            stackProperties.setProperty("gov.nist.javax.sip.CACHE_SERVER_CONNECTIONS", "true");
            stackProperties.setProperty("gov.nist.javax.sip.REJECT_STRAY_RESPONSES","true");
            stackProperties.setProperty("gov.nist.javax.sip.IS_BACK_TO_BACK_USER_AGENT", "true");
            stackProperties.setProperty("gov.nist.javax.sip.STACK_LOGGER", StackLoggerImpl.class.getName());
            stackProperties.setProperty("gov.nist.javax.sip.SERVER_LOGGER",ServerLoggerImpl.class.getName());
            stackProperties.setProperty("gov.nist.javax.sip.TLS_CLIENT_PROTOCOLS", "SSLv3, TLSv1");
            // Interval between pings ( to avoid DOS attack ).
            stackProperties.setProperty("gov.nist.javax.sip.MIN_KEEP_ALIVE_TIME_SECONDS", "1000");
            stackProperties.setProperty("gov.nist.javax.sip.RFC_2543_SUPPORT_ENABLED","false");
               
            Logger stackLogger  = Logger.getLogger(StackLoggerImpl.class);
            stackLogger.addAppender(Gateway.logAppender);
            stackLogger.setLevel(Level.toLevel(logLevel));
         
            /*
             * Break up the via encoding.
             */
            ViaList.setPrettyEncode(true);
            
            sipStack = ProtocolObjects.sipFactory.createSipStack(stackProperties);
            
            
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
