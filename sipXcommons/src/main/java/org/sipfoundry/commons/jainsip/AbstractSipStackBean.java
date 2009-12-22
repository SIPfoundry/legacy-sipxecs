package org.sipfoundry.commons.jainsip;

import gov.nist.core.StackLogger;
import gov.nist.javax.sip.SipStackExt;
import gov.nist.javax.sip.SipStackImpl;
import gov.nist.javax.sip.TransactionExt;
import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.AuthenticationHelper;
import gov.nist.javax.sip.clientauthutils.SecureAccountManager;
import gov.nist.javax.sip.header.RouteList;
import gov.nist.javax.sip.header.ViaList;

import java.util.Collection;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Properties;

import javax.sip.ClientTransaction;
import javax.sip.ListeningPoint;
import javax.sip.ObjectInUseException;
import javax.sip.SipException;
import javax.sip.SipFactory;
import javax.sip.SipListener;
import javax.sip.SipProvider;
import javax.sip.SipStack;
import javax.sip.address.AddressFactory;
import javax.sip.header.HeaderFactory;
import javax.sip.message.MessageFactory;
import javax.sip.message.Response;

import org.apache.log4j.Appender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.sipfoundry.commons.log4j.ServerLoggerImpl;
import org.sipfoundry.commons.log4j.StackLoggerImpl;
import org.sipfoundry.commons.log4j.SipFoundryLayout;


/**
 *
 * Abstract class that encapsulates a SIP stack. This initializes the stack.
 * Extend this class to effortlessly construct a SIP  stack with all the trappings we
 * need.
 */

public abstract class AbstractSipStackBean {

    private static final Logger logger = Logger.getLogger(AbstractSipStackBean.class);

    private Properties m_properties;

    private static AddressFactory m_addressFactory;

    private static HeaderFactory m_headerFactory;

    private static MessageFactory m_messageFactory;

    private SipStack m_sipStack;

    private AuthenticationHelper m_authenticationHelper = null;

    private HashMap<String,SipProvider> sipProviders = new HashMap<String,SipProvider>();
    
    private Logger m_serverLogger;

    private static SipFactory factory;
    
    static {
        try {
            factory = SipFactory.getInstance();
            factory.setPathName("gov.nist");
            m_addressFactory = factory.createAddressFactory();
            m_headerFactory = factory.createHeaderFactory();
            m_messageFactory = factory.createMessageFactory();
        }catch (Exception ex) {
            throw new RuntimeException(ex);
        } 
    }
    /**
     * Initialized stack: binds to a port. It should be called before any other operation on the
     * stack.
     */
    public void init() throws Exception {
       
        m_properties = new Properties();

        // add more properties here if needed
        m_properties.setProperty("javax.sip.STACK_NAME", getStackName() );
        m_properties.setProperty("gov.nist.javax.sip.THREAD_POOL_SIZE", "1");
        m_properties.setProperty("gov.nist.javax.sip.RECEIVE_UDP_BUFFER_SIZE", "65536");
        m_properties.setProperty("gov.nist.javax.sip.REENTRANT_LISTENER", Boolean.TRUE.toString());
        m_properties.setProperty("javax.sip.ROUTER_PATH",
                org.sipfoundry.commons.siprouter.ProxyRouter.class.getName());
        m_properties.setProperty("gov.nist.javax.sip.STACK_LOGGER", StackLoggerImpl.class.getName());
        m_properties.setProperty("gov.nist.javax.sip.SERVER_LOGGER", ServerLoggerImpl.class.getName());
        
    

        String logLevel =  SipFoundryLayout.mapSipFoundry2log4j(getLogLevel()).toString();
        m_serverLogger = Logger.getLogger(StackLoggerImpl.class);
        m_serverLogger.addAppender(getStackAppender());
        m_serverLogger.setLevel(Level.toLevel(logLevel));

        m_properties.setProperty("gov.nist.javax.sip.TRACE_LEVEL",logLevel);

        if (logLevel.equalsIgnoreCase("DEBUG")) {
            m_properties
                    .setProperty("gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND", "true");
        } else {
            m_properties.setProperty("gov.nist.javax.sip.LOG_STACK_TRACE_ON_MESSAGE_SEND",
                    "false");
        }
        
        Properties extraProperties = this.getExtraStackProperties();
        
        if ( extraProperties != null ) {
            for (Enumeration keys = extraProperties.keys(); keys.hasMoreElements(); ) {
                String key = (String) keys.nextElement();
                m_properties.setProperty(key, extraProperties.getProperty(key));
            }
        }

        try {
            m_sipStack = factory.createSipStack(m_properties);
            SipStackImpl stack = (SipStackImpl) m_sipStack;
            RouteList.setPrettyEncode(true);
            ViaList.setPrettyEncode(true);
            SipListener listener = getSipListener(this);
            for ( ListeningPointAddress hpt : this.getListeningPointAddresses()) {
              ListeningPoint listeningPoint = stack.createListeningPoint(hpt.getHost(), hpt.getPort(), hpt.getTransport());
              hpt.listeningPoint = listeningPoint;
              SipProvider sipProvider = null;
              String key = hpt.getHost() + ":" + hpt.getPort();
              if ( sipProviders.get(key) == null) {
                  sipProvider = stack.createSipProvider(listeningPoint);
                  this.sipProviders.put(key, sipProvider);
                  sipProvider.addSipListener(listener);
              } else {
                  sipProvider = this.sipProviders.get(key);
              }
              sipProvider.addListeningPoint(listeningPoint);
              hpt.sipProvider = sipProvider;
            }
            Thread.sleep(500);

            SipStackExt impl = (SipStackExt) stack;
            if ( getHashedPasswordAccountManager() != null ) {
                m_authenticationHelper = impl.getSecureAuthenticationHelper(getHashedPasswordAccountManager(), m_headerFactory);
            } else if ( getPlainTextPasswordAccountManager() != null ) {
                m_authenticationHelper = impl.getAuthenticationHelper(getPlainTextPasswordAccountManager(), m_headerFactory);
            }
          
        } catch (Exception e) {
            throw new SipxSipException("JainSip initialization exception", e);
        }
    }

    public void destroy(){
        if( m_sipStack != null ){
            try{
                Iterator lpIter = m_sipStack.getListeningPoints();
                while( lpIter.hasNext() ){
                    m_sipStack.deleteListeningPoint((ListeningPoint)(lpIter.next()));
                }
                Iterator spIter = m_sipStack.getSipProviders();
                while( spIter.hasNext() ){
                    SipProvider sp = (SipProvider)(spIter.next());
                    sp.removeSipListener(getSipListener(this));
                    m_sipStack.deleteSipProvider(sp);
                }
            }
            catch( ObjectInUseException ex ){
                m_serverLogger.error("AbstractSipStackBean::destroy caught: " + ex );
            }
        }
    }
    
    public ClientTransaction handleChallenge(Response response, ClientTransaction tid) throws SipException    {

        SipProvider sipProvider = ((TransactionExt)tid).getSipProvider();
        ClientTransaction ctx =  m_authenticationHelper.handleChallenge(response, tid,
                sipProvider, 1800);
        ctx.sendRequest();
        ctx.setApplicationData(tid.getApplicationData());
        return ctx;
    }


    public HeaderFactory getHeaderFactory() {
        return m_headerFactory;
    }

    public MessageFactory getMessageFactory() {
        return m_messageFactory;
    }

    public AddressFactory getAddressFactory() {
        return m_addressFactory;
    }


    public SipStackExt getSipStack() {
        return (SipStackExt) this.m_sipStack;
    }
    
    public StackLogger getStackLogger() {
        return ((SipStackImpl)this.m_sipStack).getStackLogger();
    }

    public ListeningPoint getListeningPoint(ListeningPointAddress hostPortTransport) {
        return hostPortTransport.listeningPoint;
    }

    public AuthenticationHelper getAuthenticationHelper() {
        return m_authenticationHelper;
    }

    public abstract String getLogLevel();
    /**
     * @return Secure Account manager or null if hashed passwords are not supported.
     */
    public abstract SecureAccountManager getHashedPasswordAccountManager();

    /**
     *
     * @return plain text password account manager or null if plain text passwords
     * not supported. Note that only one type of account is supported.
     */
    public abstract AccountManager getPlainTextPasswordAccountManager();
    public abstract SipListener getSipListener(AbstractSipStackBean abstactSipStackBean);
    public abstract String getStackName() ;
    public abstract Appender getStackAppender() ;
    public abstract Collection<ListeningPointAddress> getListeningPointAddresses();    
    public abstract Properties getExtraStackProperties();

}
