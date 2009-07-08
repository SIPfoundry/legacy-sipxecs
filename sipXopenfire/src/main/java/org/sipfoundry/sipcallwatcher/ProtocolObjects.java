package org.sipfoundry.sipcallwatcher;

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
    private static SipStack sipStack;
    static {
        try {
            sipFactory = SipFactory.getInstance();
            sipFactory.setPathName("gov.nist");
            headerFactory = sipFactory.createHeaderFactory();
            messageFactory = sipFactory.createMessageFactory();
            addressFactory = sipFactory.createAddressFactory();
            
        } catch (Exception ex) {
            throw new RuntimeException("Cannot initialize factories. Check classpath " + ex);
        }
    }

    static void init() {
        try {

            UserAgentHeader userAgentHeader = SipUtilities.createUserAgentHeader();
            ((MessageFactoryImpl) messageFactory).setDefaultUserAgentHeader(userAgentHeader);
            ServerHeader serverHeader = SipUtilities.createServerHeader();
            ((MessageFactoryImpl) messageFactory).setDefaultServerHeader(serverHeader);
              Properties stackProperties = new Properties();
            stackProperties.setProperty("javax.sip.STACK_NAME", "org.sipfoundry.sipXopenfire");
            if (!CallWatcher.getConfig().getLogLevel().equalsIgnoreCase("TRACE")) {
                if (CallWatcher.getConfig().getLogLevel().equalsIgnoreCase("DEBUG")) {
                    /*
                     * Debug level turns off stack level debug logging.
                     */
                    stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", Level.INFO
                            .toString());
                    stackProperties.setProperty(
                            "gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "true");
                } else {
                    stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", CallWatcher
                            .getConfig().getLogLevel());
                }

            } else {
                /*
                 * At TRACE level you get a LOT of logging.
                 */
                stackProperties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", Level.DEBUG
                        .toString());
            }
            stackProperties.setProperty("gov.nist.javax.sip.THREAD_POOL_SIZE", "1");
            stackProperties.setProperty("gov.nist.javax.sip.REENTRANT_LISTENER", "true");
            stackProperties.setProperty("gov.nist.javax.sip.LOG_MESSAGE_CONTENT", "true");
            stackProperties.setProperty("gov.nist.javax.sip.LOG_FACTORY",
                    SipFoundryLogRecordFactory.class.getName());
            stackProperties.setProperty("javax.sip.ROUTER_PATH",
                    org.sipfoundry.commons.siprouter.ProxyRouter.class.getName());
            /*
             * Break up the via encoding.
             */
            ViaList.setPrettyEncode(true);
            sipStack = ProtocolObjects.sipFactory.createSipStack(stackProperties);
            ((SipStackImpl) sipStack).addLogAppender(CallWatcher.getLogAppender());
        } catch (Exception ex) {
            ex.printStackTrace();
            logger.error("Error loading factories ", ex);
            throw new CallWatcherException("Error loading factories", ex);
        }

    }

    public static void start() throws SipException {
        sipStack.start();
    }

    public static void stop() {
        sipStack.stop();

    }

   

    /**
     * @return the sipStack
     */
    static SipStack getSipStack() {
        return sipStack;
    }

}
