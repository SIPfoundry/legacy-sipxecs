/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;

import javax.sip.SipProvider;
import javax.sip.address.Address;
import javax.sip.address.Hop;
import javax.sip.address.SipURI;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.siprouter.FindSipServer;
import org.sipfoundry.sipxrelay.KeepaliveMethod;
import org.sipfoundry.sipxbridge.xmlrpc.RegistrationRecord;

/**
 * The information pertaining to an ITSP account resides in this class.
 *
 * @author M. Ranganathan
 */

public class ItspAccountInfo implements
        gov.nist.javax.sip.clientauthutils.UserCredentials {
    private static Logger logger = Logger.getLogger(ItspAccountInfo.class);
    
    /**
     * The outbound proxy for the account.
     */
    private String outboundProxy;

    /**
     * The Inbound proxy for the account
     */
    private String inboundProxy;

    /**
     * The port for the inbound proxy.
     */
    private int inboundProxyPort = 5060;

    /**
     * The proxy + registrar port.
     */
    private int outboundProxyPort = 5060;

    /**
     * User name for the account.
     */
    private String userName;

    /**
     * The password for the account.
     */
    private String password;

    /**
     * The Sip Domain for the register request.
     */
    private String proxyDomain;

    /**
     * The authentication realm
     */
    private String authenticationRealm;

    /**
     * Whether or not to use stun for the contact address.
     */
    private boolean globalAddressingUsed;

    /**
     * What transport to use for signaling.
     */
    private String outboundTransport = "udp";

    /**
     * Whether or not to register on gateway initialization
     */
    private boolean registerOnInitialization = true;

    /**
     * Registration Interval (seconds)
     */
    private int registrationInterval = 600;

    /*
     * The state of the account.
     */
    private AccountState state = AccountState.INIT;

    /*
     * The call Id table.
     */
    private ConcurrentHashMap<String, FailureCounter> failureCountTable = new ConcurrentHashMap<String, FailureCounter>();

    /*
     * NAT keepalive method.
     */
    private String sipKeepaliveMethod = "CR-LF";

    /*
     * Timer task that periodically sends out CRLF
     */

    private CrLfTimerTask crlfTimerTask;

    /*
     * The Keepalive method.
     */

    private KeepaliveMethod rtpKeepaliveMethod = KeepaliveMethod.USE_DUMMY_RTP_PAYLOAD;

    /*
     * Whether or not privacy is enabled.
     */
    private boolean stripPrivateHeaders;

    /*
     * Set true if CRLF ( keepalive timer ) is started.
     */
    private boolean crLfTimerTaskStarted;

    /*
     * The asserted Identity override.
     */

    private String callerId;

    /*
     * Computed from the asserted Identity ( so we dont keep re-computing this)
     */
    private Address callerAlias;

    /*
     * If set to true use the default asserted Identity.
     */
    private boolean useDefaultAssertedIdentity;

    /*
     * Determines whether the Sip Request URI User name is a phone number.
     */
    private boolean isUserPhone = true;

    /*
     * Flag that indicates whether this is a dummy account
     */
    private boolean isDummyAccount = false;

    /*
     * The registration timer task.
     */

    protected RegistrationTimerTask registrationTimerTask;
    
    /*
     * A flag that records whether alarm has been sent for this itsp.
     */

    private boolean alarmSent;
    
    /*
     * internal flag for whether or not to use outbound proxy.
     */

    private boolean reUseOutboundProxySetting;

    /*
     * Internal flag (if true then inbound proxy port has been explicitly set).
     */
    private boolean inboundProxyPortSet;
    
    /*
     * If set to true then ITSP accepts Loose source routing.
     */

    private boolean addRoute = true;

    /*
     * The session timer interval for the ITSP.
     */
    private int sessionTimerInterval = Gateway.DEFAULT_SESSION_TIMER_INTERVAL;

    /*
     * The hop ( ip, port , transport ) to the ITSP registrar.
     */
    private Hop hopToRegistrar ;

    /*
     * Whether or not this is enabled.
     */
    private boolean enabled = true;

    private String configuredOutboundProxy;

    /*
     * This scans the failure counter table and removes records after a timeout period.
     */
    class FailureCounterScanner extends TimerTask {

        public FailureCounterScanner() {

        }

        @Override
        public void run() {

            try {
                for (Iterator<FailureCounter> it = failureCountTable.values()
                        .iterator(); it.hasNext();) {
                    FailureCounter fc = it.next();
                    long now = System.currentTimeMillis();
                    if (now - fc.creationTime > 30000) {
                        it.remove();
                    }
                }
            } catch (Exception ex) {
                logger.error("An exception occured in the timer task ",ex);
            }
        }

    }

    class FailureCounter {
        long creationTime;
        int counter;

        FailureCounter() {
            creationTime = System.currentTimeMillis();
            counter = 0;
        }

        int increment() {
            counter++;
            return counter;
        }
    }

    public ItspAccountInfo() {

    }

    public void startFailureCounterScanner() {
        FailureCounterScanner fcs = new FailureCounterScanner();
        Gateway.getTimer().schedule(fcs, 5000, 5000);
    }

    public String getOutboundProxy() {
        if (!reUseOutboundProxySetting) {
            this.lookupAccount();
            return outboundProxy;
        } else if (this.outboundProxy != null) {
            return outboundProxy;
        } else {
            return this.getSipDomain();
        }
    }
    
    private String getConfiguredOutboundProxy() {
        return this.configuredOutboundProxy == null ? this.proxyDomain : configuredOutboundProxy;
    }
    

    public int getOutboundProxyPort() {
        return outboundProxyPort;
    }

    public String getUserName() {
        return userName;
    }

    public String getPassword() {
        return password;
    }

    public String getSipDomain() {
        return proxyDomain;
    }

    public boolean isGlobalAddressingUsed() {
        return this.globalAddressingUsed;
    }

    public void setOutboundProxy(String resolvedName) {
        this.outboundProxy = resolvedName;
        this.configuredOutboundProxy = resolvedName;
        this.reUseOutboundProxySetting = true;

    }

    public void lookupAccount() {
        // User has already specified an outbound proxy so just bail out.
        if (this.outboundProxy != null && reUseOutboundProxySetting) {
            return;
        }
        try {
            String outboundDomain = this.getSipDomain();
            SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(null,
                    outboundDomain);
            sipUri.setTransportParam(this.outboundTransport);
            Hop hop = new FindSipServer(logger).findServer(sipUri);
           
            if ( this.outboundProxyPort != 5060 ) {
              this.setOutboundProxyPort(hop.getPort());
            }
            this.outboundProxy = hop.getHost();
            this.reUseOutboundProxySetting = false;
        } catch (Exception ex) {
            logger.error(
                    "Exception in processing -- could not add ITSP account ",
                    ex);
        }
    }

    public int getRegistrationInterval() {
        return registrationInterval;
    }

    public void setRegistrationInterval(int registrationInterval) {
        this.registrationInterval = registrationInterval;
    }

    /**
     * @param proxyPort
     *            the proxyPort to set
     */
    public void setOutboundProxyPort(int proxyPort) {
        /* 0 means default */
        if (proxyPort != 0)
            this.outboundProxyPort = proxyPort;
    }

    /**
     * @param password
     *            the password to set
     */
    public void setPassword(String password) {
        this.password = password;
    }

    /**
     * @param proxyDomain
     *            the proxyDomain to set
     */
    public void setProxyDomain(String proxyDomain) {
        this.proxyDomain = proxyDomain;
    }

    /**
     * @return the proxyDomain
     */
    public String getProxyDomain() {
        return proxyDomain;
    }

    /**
     * @param authenticationRealm
     *            the authenticationRealm to set
     */
    public void setAuthenticationRealm(String authenticationRealm) {
        this.authenticationRealm = authenticationRealm;
    }

    /**
     * Note that authentication realm is an optional configuration parameter. We
     * return the authenticationRealm if set else we return the outbound proxy
     * if set else we return the domain.
     *
     * @return the authenticationRealm
     */
    public String getAuthenticationRealm() {
        if (authenticationRealm != null) {
            return authenticationRealm;
        } else {
            return proxyDomain;
        }
    }

    /**
     * @param outboundTransport
     *            the outboundTransport to set
     */
    public void setOutboundTransport(String outboundTransport) {
        this.outboundTransport = outboundTransport;
    }

    /**
     * @return the outboundTransport
     */
    public String getOutboundTransport() {
        return outboundTransport;
    }

    /**
     * @param registerOnInitialization
     *            the registerOnInitialization to set
     */
    public void setRegisterOnInitialization(boolean registerOnInitialization) {
        this.registerOnInitialization = registerOnInitialization;
    }

    /**
     * @return the registerOnInitialization
     */
    public boolean isRegisterOnInitialization() {
        return registerOnInitialization;
    }

    /**
     * @param globalAddressingUsed
     *            the globalAddressingUsed to set
     */
    public void setGlobalAddressingUsed(boolean globalAddressingUsed) {
        this.globalAddressingUsed = globalAddressingUsed;
    }

    /**
     * @param userName
     *            the userName to set
     */
    public void setUserName(String userName) {
        this.userName = userName;
    }

    public int incrementFailureCount(String callId) {
        FailureCounter fc = this.failureCountTable.get(callId);
        if (fc == null) {
            fc = new FailureCounter();
            this.failureCountTable.put(callId, fc);
        }
        int retval = fc.increment();
        logger.debug("incrementFailureCount : " + callId + " currentCount = " + retval);

        return retval;

    }


    /**
     * @param state
     *            the state to set
     */
    public void setState(AccountState state) {
        this.state = state;
    }

    /**
     * @return the state
     */
    public AccountState getState() {
        return state;
    }

    /**
     * @param sipKeepaliveMethod
     *            the sipKeepaliveMethod to set
     */
    public void setSipKeepaliveMethod(String sipKeepaliveMethod) {
        this.sipKeepaliveMethod = sipKeepaliveMethod;
    }

    /**
     * @return the sipKeepaliveMethod
     */
    public String getSipKeepaliveMethod() {
        return sipKeepaliveMethod;
    }

    public void startCrLfTimerTask() {
        if (!this.crLfTimerTaskStarted) {

            this.crLfTimerTaskStarted = true;
            this.crlfTimerTask = new CrLfTimerTask(Gateway
                    .getWanProvider("udp"), this);
            logger.debug("ItspAccountInfo: startCrLfTimerTask() : "
                    + this.getProxyDomain() + " keepalive = "
                    + Gateway.getSipKeepaliveSeconds());
            Gateway.getTimer().schedule(crlfTimerTask,
                    Gateway.getSipKeepaliveSeconds() * 1000,
                    Gateway.getSipKeepaliveSeconds() * 1000);
        }

    }

    public void stopCrLfTimerTask() {
        if (this.crLfTimerTaskStarted) {
            logger.debug("ItspAccountInfo: stopCrLfTimerTask() : "
                    + this.getProxyDomain());
            this.crLfTimerTaskStarted = false;
            this.crlfTimerTask.cancel();

        }
    }

    /**
     * @param rtpKeepaliveMethod
     *            the rtpKeepaliveMethod to set
     */
    public void setRtpKeepaliveMethod(String rtpKeepaliveMethod) {
        this.rtpKeepaliveMethod = KeepaliveMethod
                .valueOfString(rtpKeepaliveMethod);
    }

    /**
     * @return the rtpKeepaliveMethod
     */
    public KeepaliveMethod getRtpKeepaliveMethod() {
        return rtpKeepaliveMethod;
    }

    /**
     * @return the outboundRegistrarRoute
     */
    public String getOutboundRegistrar() {
        return this.getInboundProxy();
    }

    /**
     * @param inboundProxy
     *            the inboundProxy to set
     */
    public void setInboundProxy(String inboundProxy) {
        this.inboundProxy = inboundProxy;
     }

    /**
     * @return the inboundProxy
     */
    public String getInboundProxy() {
        return inboundProxy == null ? getOutboundProxy() : inboundProxy;
    }
    
    /**
     * @return the inboundProxy addresses
     */
    public Collection<Hop> getInboundProxies() {
        try {
            String inBoundProxyDomain = this.getConfiguredInboundProxy();
            int inBoundProxyPort = this.getInboundProxyPort();
            SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(null, inBoundProxyDomain);
            if ( inBoundProxyPort != 5060) {
                sipUri.setPort(inBoundProxyPort);
            }
            Collection<Hop> hops = new org.sipfoundry.commons.siprouter.FindSipServer(logger).findSipServers(sipUri);
            return hops;
        } catch ( Exception ex) {
            return null;
        }

    }
    
    /**
     * @return the configured inboundProxy
     */
    private String getConfiguredInboundProxy() {
        return this.inboundProxy == null ? getConfiguredOutboundProxy() : inboundProxy;
    }

    /**
     * @return the inbound proxy port.
     */
    public int getInboundProxyPort() {
        return inboundProxy != null ? inboundProxyPort : this
                .getOutboundProxyPort();
    }

    public void setInboundProxyPort(int port) {
        if (port < 0) {
            throw new IllegalArgumentException("Bad inbound proxy port " + port);
        } else if (port > 0) {
            this.inboundProxyPortSet = true;
            this.inboundProxyPort = port;
        }
    }

    /**
     * @return the registration record for this account
     */
    public RegistrationRecord getRegistrationRecord() {
        return (new RegistrationRecord(this.getProxyDomain(), this.state.toString()));
    }

    public void setCallerId(String callerId) {
        this.callerId = callerId;
    }

    public String getDefaultDomain() {
        if (this.isRegisterOnInitialization()) {
            return  this.getProxyDomain();
        } else if (this.isGlobalAddressingUsed() && this.getUserName() != null ) {
            return  Gateway.getGlobalAddress();
        } else if (this.getUserName() != null ){
            return  Gateway.getLocalAddress();
        } else {
            return null;
        }
    }


    public String getCallerId() {
        if (!useDefaultAssertedIdentity) {
            return this.callerId;
        } else if (this.getDefaultDomain() != null ) {
            return this.getUserName() + "@" + this.getDefaultDomain();
        } else {
            return null;
        }

    }

    protected Address getCallerAlias() {
        try {
            if (this.callerAlias != null) {
                return this.callerAlias;
            } else {
                if (this.getCallerId() != null) {
                    String callerId = "sip:" + this.getCallerId();
                    SipURI sipUri = (SipURI) ProtocolObjects.addressFactory
                            .createURI(callerId);

                    this.callerAlias = ProtocolObjects.addressFactory
                            .createAddress(sipUri);
                    return this.callerAlias;
                } else {
                    return null;
                }
            }
        } catch (Exception ex) {
            return null;
        }
    }

    /**
     * @param stripPrivateHeaders
     *            the stripPrivateHeaders to set
     */
    public void setStripPrivateHeaders(boolean stripPrivateHeaders) {
        this.stripPrivateHeaders = stripPrivateHeaders;
    }

    /**
     * @return the stripPrivateHeaders
     */
    public boolean stripPrivateHeaders() {
        return stripPrivateHeaders;
    }

    /**
     * set the flag to use default asserted identity.
     *
     * @param flag
     */
    public void setUseDefaultAssertedIdentity(boolean flag) {
        this.useDefaultAssertedIdentity = flag;
    }

    /**
     * Remove a failure counter.
     *
     * @param callId
     */
    public void removeFailureCounter(String callId) {
        logger.debug("removeFailureCounter " + callId);
        this.failureCountTable.remove(callId);
    }

    /**
     * @param param
     *            to set.
     */
    public void setUserPhone(boolean isUserPhone) {
        this.isUserPhone = isUserPhone;
    }

    /**
     * @return the isUserPhone
     */
    public boolean isUserPhone() {
        return isUserPhone;
    }

    public void setAlarmSent(boolean b) {
        this.alarmSent = b;
    }

    public boolean isAlarmSent() {
        return alarmSent;
    }

    public boolean isDummyAccount() {
        return this.isDummyAccount;
    }

    public void setDummyAccount(boolean dummyAccount) {
        this.isDummyAccount = dummyAccount;
    }

    public Collection<Hop> getItspProxyAddresses() {
        try {
            if (this.outboundProxy != null && reUseOutboundProxySetting) {
                Collection<Hop> retval = new HashSet<Hop>();
                Hop hop = new HopImpl(this.getOutboundProxy(), this
                        .getOutboundProxyPort(), this.getOutboundTransport());
                retval.add(hop);
                return retval;
            } else {
                String outboundDomain = this.getSipDomain();
                SipURI sipUri = ProtocolObjects.addressFactory.createSipURI(
                        null, outboundDomain);
                if ( this.outboundProxyPort != 5060 ) {
                    sipUri.setPort(this.outboundProxyPort);
                }
                return new FindSipServer(logger).findSipServers(sipUri);
            }
        } catch (Exception ex) {
            throw new SipXbridgeException(ex);
        }
    }



    /**
     * @return the inboundProxyPortSet
     */
    public boolean isInboundProxyPortSet() {
        return inboundProxyPortSet;
    }

    public boolean isAddLrRoute() {
       return this.addRoute;
    }

    /**
     * Set the SIP session timer interval.
     *
     * @param sessionTimerInterval
     */
    public void setSipSessionTimerIntervalSeconds(int sessionTimerInterval) {
        this.sessionTimerInterval = sessionTimerInterval;
    }


    /**
     * @return the sessionTimerInterval
     */
    public int getSessionTimerInterval() {
        return sessionTimerInterval;
    }

    /**
     * @param addRoute the addRoute to set
     */
    public void setAddLrRoute(boolean addRoute) {
        this.addRoute = addRoute;
    }

    /**
     * Set the hop to Registrar ( if account needs it )
     * @param hop
     */
    public void setHopToRegistrar(Hop hop) {
       this.hopToRegistrar = hop;
    }

    /**
     * Get hop to Registrar.
     * 
     * @return
     */
    public Hop getHopToRegistrar() {
        return this.hopToRegistrar;
    }
    
    /**
     * Set the enabled flag.
     * 
     * @param flag
     */
    public void setEnabled(boolean flag) {
        this.enabled = flag;       
    }
    
    /**
     * Get the enabled flag.
     * 
     * @return
     */
    public boolean isEnabled() {
        return this.enabled;
    }

    public SipProvider getSipProvider() {
        logger.debug("Provider = " + Gateway.getWanProvider(this.outboundTransport.toLowerCase()) );
        return Gateway.getWanProvider(this.outboundTransport.toLowerCase());
    }
}
