/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.net.InetAddress;
import java.util.HashMap;
import java.util.Iterator;
import java.util.TimerTask;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxbridge.symmitron.KeepaliveMethod;
import org.sipfoundry.sipxbridge.xmlrpc.RegistrationRecord;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.Record;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.TextParseException;
import org.xbill.DNS.Type;

/**
 * The information pertaining to an ITSP account resides in this class.
 * 
 * @author M. Ranganathan
 */

public class ItspAccountInfo implements gov.nist.javax.sip.clientauthutils.UserCredentials {
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
     * The outbound Registrar
     */
    private String outboundRegistrar;
    
    /**
     * The outbound registrar port.
     */
    private int outboundRegistrarPort = 5060;

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
     * Use rport setting ( if stun is not used )
     */
    private boolean rportUsed = false;

   
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
    private HashMap<String, FailureCounter> failureCountTable = new HashMap<String, FailureCounter>();

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
     * Whether or not to use registration for caller id when sending out invite.
     */

    private boolean useRegistrationForCallerId = true;
     
    /*
     * Number of concurrent calls.
     */

    private int maxCalls = -1;

    private int callCount = 0;

   

    private boolean crLfTimerTaskStarted;

    protected  RegistrationTimerTask registrationTimerTask;
    
    private String callerId;
    
    
    /**
     * This task runs periodically depending upon the timeout of the lookup specified.
     * 
     */
    class Scanner extends TimerTask {

        public void run() {
            try {
                Record[] records = new Lookup("_sip._" + getOutboundTransport() + "."
                        + getSipDomain(), Type.SRV).run();
                logger.debug("Did a successful DNS SRV lookup");
                SRVRecord record = (SRVRecord) records[0];
                int port = record.getPort();
                setPort(port);
                long time = record.getTTL() * 1000;
                String resolvedName = record.getTarget().toString();
                setOutboundProxy(resolvedName);
                HopImpl proxyHop = new HopImpl(InetAddress.getByName(getOutboundProxy())
                        .getHostAddress(), getProxyPort(), getOutboundTransport(),
                        ItspAccountInfo.this);
                Gateway.getAccountManager().setHopToItsp(getSipDomain(), proxyHop);
                Gateway.getTimer().schedule(new Scanner(), time);
            } catch (Exception ex) {
                logger.error("Error looking up domain " + "_sip._" + getOutboundTransport() + "."
                        + getSipDomain());
            }

        }

    }

    class FailureCounterScanner extends TimerTask {

        public FailureCounterScanner() {

        }

        @Override
        public void run() {

            for (Iterator<FailureCounter> it = failureCountTable.values().iterator(); it
                    .hasNext();) {
                FailureCounter fc = it.next();
                long now = System.currentTimeMillis();
                if (now - fc.creationTime > 30000) {
                    it.remove();
                }
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
        if (outboundProxy != null)
            return outboundProxy;
        else
            return this.getSipDomain();

    }

    public int getProxyPort() {
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

    public boolean isRportUsed() {
        return this.rportUsed;
    }

    /**
     * Set the proxy port
     * 
     * @param port
     */

    public void setPort(int port) {
        this.outboundProxyPort = port;

    }

    public void setOutboundProxy(String resolvedName) {
         this.outboundProxy = resolvedName;

    }

    public void startDNSScannerThread(long time) {
        Gateway.getTimer().schedule(new Scanner(), time);

    }

    public void lookupAccount() throws TextParseException,GatewayConfigurationException {
        // User has already specified an outbound proxy so just bail out.
        if (this.outboundProxy != null)
            return;
        try {
            String outboundDomain = this.getSipDomain();
            Record[] records = new Lookup("_sip._" + this.getOutboundTransport() + "."
                    + outboundDomain, Type.SRV).run();

            if (records == null || records.length == 0) {
                // SRV lookup failed, use the outbound proxy directly.
                logger
                        .debug("SRV lookup returned nothing -- we are going to just use the domain name directly");
            } else {
                logger.debug("Did a successful DNS SRV lookup");
                SRVRecord record = (SRVRecord) records[0];
                int port = record.getPort();
                this.setPort(port);
                long time = record.getTTL() * 1000;
                String resolvedName = record.getTarget().toString();
                if (resolvedName.endsWith(".")) {
                    resolvedName = resolvedName.substring(0, resolvedName.lastIndexOf('.'));
                }
                this.setOutboundProxy(resolvedName);
                this.startDNSScannerThread(time);
            }

        } catch (TextParseException ex) {
                throw ex;
        } catch (Exception ex) {
            
            logger.fatal("Exception in processing -- could not add ITSP account ", ex);
            throw new GatewayConfigurationException("Problem with domain name lookup", ex);
        }
    }

   

    public int getRegistrationInterval() {
        return registrationInterval;
    }
    
    public void setRegistrationInterval( int registrationInterval ) {
        this.registrationInterval = registrationInterval;
    }

    /**
     * @param proxyPort the proxyPort to set
     */
    public void setProxyPort(int proxyPort) {
        this.outboundProxyPort = proxyPort;
    }

   
    /**
     * @param password the password to set
     */
    public void setPassword(String password) {
        this.password = password;
    }

    /**
     * @param proxyDomain the proxyDomain to set
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
     * @param authenticationRealm the authenticationRealm to set
     */
    public void setAuthenticationRealm(String authenticationRealm) {
         this.authenticationRealm = authenticationRealm;
    }

    /**
     * Note that authentication realm is an optional configuration parameter. We return the
     * authenticationRealm if set else we return the outbound proxy if set else we return the
     * domain.
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
     * @param outboundTransport the outboundTransport to set
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
     * @param registerOnInitialization the registerOnInitialization to set
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
     * @param rportUsed the rportUsed to set
     */
    public void setRportUsed(boolean rportUsed) {
        this.rportUsed = rportUsed;
    }

    /**
     * @param globalAddressingUsed the globalAddressingUsed to set
     */
    public void setGlobalAddressingUsed(boolean globalAddressingUsed) {
        this.globalAddressingUsed = globalAddressingUsed;
    }

    /**
     * @param userName the userName to set
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
        logger.debug("incrementFailureCount : " + retval);

        return retval;

    }

    /**
     * @param state the state to set
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
     * @param sipKeepaliveMethod the sipKeepaliveMethod to set
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
            this.crlfTimerTask = new CrLfTimerTask(Gateway.getWanProvider("udp"), this);
            Gateway.getTimer().schedule(crlfTimerTask, Gateway.getSipKeepaliveSeconds() * 1000);
        }

    }
    
    public void stopCrLfTimerTask() {
        if ( this.crLfTimerTaskStarted ) {
            this.crLfTimerTaskStarted  = false;
            this.crlfTimerTask.cancel();
            
        }
    }

    /**
     * @param rtpKeepaliveMethod the rtpKeepaliveMethod to set
     */
    public void setRtpKeepaliveMethod(String rtpKeepaliveMethod) {
        this.rtpKeepaliveMethod = KeepaliveMethod.valueOfString(rtpKeepaliveMethod);
    }

    /**
     * @return the rtpKeepaliveMethod
     */
    public KeepaliveMethod getRtpKeepaliveMethod() {
        return rtpKeepaliveMethod;
    }

    /**
     * @param useRegistrationForCallerId the useRegistrationForCallerId to set
     */
    public void setUseRegistrationForCallerId(boolean useRegistrationForCallerId) {
        this.useRegistrationForCallerId = useRegistrationForCallerId;
    }

    

    /**
     * @param maxCalls the maxCalls to set
     */
    public void setMaxCalls(String maxCalls) {
        try {
            this.maxCalls = Integer.parseInt(maxCalls);
        } catch (NumberFormatException ex) {
            logger.error("Illegal Argument " + maxCalls);

        }
    }

    /**
     * @return the maxCalls
     */
    public int getMaxCalls() {
        return maxCalls;
    }

    public void decrementCallCount() {
        if (this.callCount > 0) {
            this.callCount--;
        }

    }

    public int getCallCount() {
        return this.callCount;
    }

    public void incrementCallCount() {
        this.callCount++;

    }

    

    
    /**
     * @param outboundRegistrarRoute the outboundRegistrarRoute to set
     */
    public void setOutboundRegistrar(String outboundRegistrarRoute) {
        this.outboundRegistrar = outboundRegistrarRoute;
    }

    /**
     * @return the outboundRegistrarRoute
     */
   public String getOutboundRegistrar() {
        return this.getInboundProxy();
    }
    
    

    /**
     * @param inboundProxy the inboundProxy to set
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
     * @return the inbound proxy port.
     */
    public int getInboundProxyPort() {
        return inboundProxy != null ? inboundProxyPort : this.getProxyPort();
    }
    
    
    public void setInboundProxyPort(String portString) {
        try {
            int port = Integer.parseInt(portString);
            this.inboundProxyPort = port;
        } catch (NumberFormatException ex) {
            logger.error("Invalid number format " + portString);
        }
        
    }
    /**
     * @return the registration record for this account
     */
    public RegistrationRecord getRegistrationRecord() {
        RegistrationRecord retval = new RegistrationRecord();
        retval.setRegisteredAddress(this.getProxyDomain());
        retval.setRegistrationStatus(this.state.toString());
        return retval;
    }

   
    
    public void setCallerId(String callerId) {
        this.callerId = callerId;
    }
    public String getCallerId() {
        if ( this.callerId != null ) {
            return this.callerId;
        } else if (this.isRegisterOnInitialization()) {
            return this.getUserName() + "@" + this.getProxyDomain();
        } else  {
            return  this.getUserName() + "@" + Gateway.getGlobalAddress();
        } 
       
    }

}
