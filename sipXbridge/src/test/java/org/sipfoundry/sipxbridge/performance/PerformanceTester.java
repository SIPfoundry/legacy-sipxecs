package org.sipfoundry.sipxbridge.performance;

import java.util.Properties;
import java.util.Random;
import java.util.Timer;

import javax.sdp.SdpFactory;
import javax.sdp.SessionDescription;
import javax.sip.Dialog;
import javax.sip.PeerUnavailableException;
import javax.sip.SipFactory;
import javax.sip.address.AddressFactory;
import javax.sip.header.Header;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.log4j.ServerLoggerImpl;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.commons.log4j.SipFoundryLogRecordFactory;
import org.sipfoundry.commons.log4j.StackLoggerImpl;
import org.sipfoundry.sipxbridge.AccountManagerImpl;
import org.sipfoundry.sipxbridge.BridgeConfiguration;
import org.sipfoundry.sipxbridge.ConfigurationParser;
import org.sipfoundry.sipxbridge.Gateway;
import org.sipfoundry.sipxbridge.ItspAccountInfo;

import junit.framework.TestCase;

import gov.nist.javax.sip.ListeningPointExt;
import gov.nist.javax.sip.SipProviderExt;
import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.header.HeaderFactoryExt;
import gov.nist.javax.sip.message.MessageFactoryExt;

public class PerformanceTester {

    private static SipStackExt userAgentStack;
    private static SipStackExt itspStack;
  
    static Logger logger = Logger.getLogger(PerformanceTester.class);

    static HeaderFactoryExt headerFactory;
    static MessageFactoryExt messageFactory;
    static AddressFactory addressFactory;

    static Timer timer = new Timer();

    static int RING_TIME = 100;
    static int CALL_TIME = 2 * 1000;
    
    static long startTime  = System.currentTimeMillis() ;
  
    static int itspCompletedCalls;

    private static int baseMediaPort = 20000;
    
    private static int counter = 0;

    static BridgeConfiguration bridgeConfiguration;
    public static int phoneDialogCompletions;

    private static String sdpBodyFormat = "v=0\r\n" + "o=- %s 1 IN IP4 %s\r\n"
            + "s=iBridge\r\n" + "c=IN IP4 %s\r\n" + "t=0 0\r\n"
            + "a=direction:active\r\n"
            + "m=audio %d RTP/AVP 3 97 98 110 8 0 101\r\n"
            + "a=rtpmap:3 GSM/8000\r\n" + "a=rtpmap:97 iLBC/8000\r\n"
            + "a=rtpmap:98 iLBC/8000\r\n" + "a=fmtp:98 mode=20\r\n"
            + "a=rtpmap:110 speex/8000\r\n" + "a=rtpmap:8 PCMA/8000\r\n"
            + "a=rtpmap:0 PCMU/8000\r\n"
            + "a=rtpmap:101 telephone-event/8000\r\n"
            + "a=fmtp:101 0-11,16\r\n";
   static ItspAccountInfo itspAccountInfo;

    public static int getMediaPort() {
        return baseMediaPort + counter++ % 1000;
    }

    static SessionDescription createSessionDescription(String ipAddress) {
        try {
            int mediaPort = getMediaPort();
            String origin = Long.toString(Math.abs(new Random().nextLong()));

            String sdpBody = String.format(sdpBodyFormat, origin, ipAddress,
                    ipAddress, mediaPort);
            SessionDescription sd = SdpFactory.getInstance()
                    .createSessionDescription(sdpBody);

            return sd;

        } catch (Exception ex) {
            TestCase.fail("Cannot create session description");
            return null;
        }

    }

    public static void main(String[] args ) throws Exception {
        
        System.out.println("Performance tester");
        SipFactory sipFactory = null;
        sipFactory = SipFactory.getInstance();
        sipFactory.setPathName("gov.nist");
        addressFactory = sipFactory.createAddressFactory();
        headerFactory = (HeaderFactoryExt) sipFactory.createHeaderFactory();
        messageFactory = (MessageFactoryExt) sipFactory.createMessageFactory();
       
        
        String confDir = System.getProperties().getProperty("conf.dir","/usr/local/sipx/etc/sipxpbx/");
        
        String confFile = confDir.equals("./")? "file:sipxbridge.xml" : "file:///" + confDir + "/" + "sipxbridge.xml";
        
        System.out.println("confFile = "+ confFile);
        
        ConfigurationParser configFileParser = new ConfigurationParser();
        AccountManagerImpl accountManager = configFileParser.createAccountManager(confFile);
        bridgeConfiguration = accountManager.getBridgeConfiguration();
        itspAccountInfo = accountManager.getDefaultAccount();
        SipFoundryAppender logAppender = new SipFoundryAppender(
                new SipFoundryLayout(), "siptester.log",true);
        
        Logger logger = Logger.getLogger(PerformanceTester.class.getPackage().getName());
        logger.addAppender(logAppender);
        
        
        Logger stackLogger  = Logger.getLogger(StackLoggerImpl.class);
        stackLogger.addAppender(logAppender);
        stackLogger.setLevel(Level.toLevel(bridgeConfiguration.getLogLevel()));
        
        logger.info("confFile = " + confFile);
        
        try {
            // Create SipStack object
            Properties properties = new Properties();
            // If you want to try TCP transport change the following to
            // Drop the client connection after we are done with the transaction.
            properties.setProperty("gov.nist.javax.sip.CACHE_CLIENT_CONNECTIONS",
                    "true");
            properties.setProperty("gov.nist.javax.sip.CACHE_SERVER_CONNECTIONS",
            "true");
            properties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", bridgeConfiguration.getLogLevel());
            // If you want to use UDP then uncomment this.
            properties.setProperty("javax.sip.STACK_NAME", "org.sipfoundry.sipxbridge.performance.phone");
            String logLevel = bridgeConfiguration.getLogLevel();
            properties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", bridgeConfiguration.getLogLevel());
            if (logLevel.equalsIgnoreCase("DEBUG") || logLevel.equalsIgnoreCase("TRACE")) {
                properties.setProperty("gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "true");
            } else {
                properties.setProperty("gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "false");
            }
            properties.setProperty("gov.nist.javax.sip.STACK_LOGGER", StackLoggerImpl.class.getName());
            properties.setProperty("gov.nist.javax.sip.SERVER_LOGGER",ServerLoggerImpl.class.getName());
         
            properties.setProperty("gov.nist.javax.sip.LOG_FACTORY",
                    SipFoundryLogRecordFactory.class.getName());
            userAgentStack = (SipStackExt)sipFactory.createSipStack(properties);
            ListeningPointExt userAgentListeningPoint  = (ListeningPointExt)userAgentStack.createListeningPoint(bridgeConfiguration.getSipxProxyDomain(), 
                    bridgeConfiguration.getSipxProxyPort(), "tcp");
            ListeningPointExt udpUserAgentListeningPoint = (ListeningPointExt)userAgentStack.createListeningPoint(bridgeConfiguration.getSipxProxyDomain(), 
                    bridgeConfiguration.getSipxProxyPort(), "udp");
            SipProviderExt sipProvider = (SipProviderExt) userAgentStack.createSipProvider(userAgentListeningPoint);
            sipProvider.addListeningPoint(udpUserAgentListeningPoint);
            PhoneListener phoneListener = new PhoneListener( userAgentListeningPoint, sipProvider);
            sipProvider.addSipListener(phoneListener);
            // Create SipStack object
             properties = new Properties();
            // If you want to try TCP transport change the following to
            // Drop the client connection after we are done with the transaction.
            properties.setProperty("gov.nist.javax.sip.CACHE_CLIENT_CONNECTIONS",
                    "true");
            properties.setProperty("gov.nist.javax.sip.CACHE_SERVER_CONNECTIONS",
            "true");
          
         
            properties.setProperty("gov.nist.javax.sip.TRACE_LEVEL", bridgeConfiguration.getLogLevel());
            if (logLevel.equalsIgnoreCase("DEBUG") || logLevel.equalsIgnoreCase("TRACE")) {
                properties.setProperty("gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "true");
            } else {
                properties.setProperty("gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "false");
            }
            properties.setProperty("gov.nist.javax.sip.LOG_FACTORY",
                    SipFoundryLogRecordFactory.class.getName());
            properties.setProperty("gov.nist.javax.sip.STACK_LOGGER", StackLoggerImpl.class.getName());
            properties.setProperty("gov.nist.javax.sip.SERVER_LOGGER",ServerLoggerImpl.class.getName());
          
            // If you want to use UDP then uncomment this.
            properties.setProperty("javax.sip.STACK_NAME", "org.sipfoundry.sipxbridge.performance.itsp");
            itspStack = (SipStackExt)sipFactory.createSipStack(properties);
             ListeningPointExt itspListeningPoint  = (ListeningPointExt)userAgentStack.createListeningPoint(itspAccountInfo.getOutboundProxy(), 
                    itspAccountInfo.getOutboundProxyPort(), itspAccountInfo.getOutboundTransport());
             SipProviderExt itspProvider = (SipProviderExt) itspStack.createSipProvider(itspListeningPoint);
                 
            ItspListener itspListener = new ItspListener(itspListeningPoint,itspProvider);
            itspProvider.addSipListener(itspListener);
            System.out.println("Sending INVITE");
            for ( int i = 0; i < 1000000 ; i++) {
                phoneListener.sendInvite();
                Thread.sleep(150);
            }
            
            Thread.sleep(120*1000);
            System.out.println("Done!");
            System.exit(0);
         
        } catch (Exception e) {
            // could not find
            // gov.nist.jain.protocol.ip.sip.SipStackImpl
            // in the classpath
            e.printStackTrace();
            System.err.println(e.getMessage());
            System.exit(0);
        }


  
    }

}
