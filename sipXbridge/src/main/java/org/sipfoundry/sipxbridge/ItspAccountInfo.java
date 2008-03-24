/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.net.InetAddress;
import java.util.TimerTask;

import javax.sip.address.SipURI;

import org.apache.log4j.Logger;
import org.xbill.DNS.Lookup;
import org.xbill.DNS.Record;
import org.xbill.DNS.SRVRecord;
import org.xbill.DNS.Type;

/**
 * The information pertaining to an ITSP account resides in this class.
 * 
 * @author M. Ranganathan
 */

public class ItspAccountInfo implements
        gov.nist.javax.sip.clientauthutils.UserCredentials {

    private static Logger logger = Logger.getLogger(ItspAccountInfo.class);

    /**
     * The proxy + registrar for the account.
     */
    private String outboundProxy;

    /**
     * The proxy + registrar port.
     */
    private int proxyPort = 5060;

    /**
     * User name for the account.
     */
    private String userName;

    /**
     * The display name for the account.
     */
    private String displayName;

    /**
     * The display name for the account.
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
     * Route the request to the Auto attendant.
     */
    private boolean inboundCallsRoutedToAutoAttendant = true;

    /**
     * Whether or not to register on gateway initialization
     */
    private boolean registerOnInitialization = true;

    /**
     * The options timer task.
     */
    private OptionsTimerTask optionsTimerTask;

    /**
     * Registration Interval (seconds)
     */
    private int registrationInterval = 90;

    /**
     * This task runs periodically depending upon the timeout of the lookup
     * specified.
     * 
     */
    class Scanner extends TimerTask {

        public void run() {
            try {
                Record[] records = new Lookup("_sip._" + getTransport() + "."
                        + getSipDomain(), Type.SRV).run();
                logger.debug("Did a successful DNS SRV lookup");
                SRVRecord record = (SRVRecord) records[0];
                int port = record.getPort();
                setPort(port);
                long time = record.getTTL() * 1000;
                String resolvedName = record.getTarget().toString();
                setOutboundProxy(resolvedName);
                HopImpl proxyHop = new HopImpl(InetAddress.getByName(
                        getOutboundProxy()).getHostAddress(), getProxyPort(),
                        getTransport());
                Gateway.getAccountManager().setHopToItsp(getSipDomain(),
                        proxyHop);
                Gateway.timer.schedule(new Scanner(), time);
            } catch (Exception ex) {
                Gateway.stop();
            }

        }

    }

    public ItspAccountInfo() {

    }

    public String getOutboundProxy() {
        return outboundProxy;
    }

    public int getProxyPort() {
        return proxyPort;
    }

    public String getUserName() {
        return userName;
    }

    public String getDisplayName() {
        return displayName;
    }

    public String getPassword() {
        return password;
    }

    public String getSipDomain() {
        return proxyDomain;
    }

    public String getTransport() {
        return this.outboundTransport;
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
        this.proxyPort = port;

    }

    public void setOutboundProxy(String resolvedName) {
        this.outboundProxy = resolvedName;

    }

    public void startDNSScannerThread(long time) {
        Gateway.timer.schedule(new Scanner(), time);

    }

    public void setOptionsTimerTask(OptionsTimerTask optionsTimerTask) {
        this.optionsTimerTask = optionsTimerTask;
    }

    public OptionsTimerTask getOptionsTimerTask() {
        return optionsTimerTask;
    }

    public boolean isInboundCallsRoutedToAutoAttendant() {
        return inboundCallsRoutedToAutoAttendant;
    }

    public int getRegistrationInterval() {
        return registrationInterval;
    }

    /**
     * @param proxyPort
     *            the proxyPort to set
     */
    public void setProxyPort(int proxyPort) {
        this.proxyPort = proxyPort;
    }

    /**
     * @param displayName
     *            the displayName to set
     */
    public void setDisplayName(String displayName) {
        this.displayName = displayName;
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
     * @return the authenticationRealm
     */
    public String getAuthenticationRealm() {
        return authenticationRealm;
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
     * @param rportUsed
     *            the rportUsed to set
     */
    public void setRportUsed(boolean rportUsed) {
        this.rportUsed = rportUsed;
    }

    /**
     * @param inboundCallsRoutedToAutoAttendant
     *            the inboundCallsRoutedToAutoAttendant to set
     */
    public void setInboundCallsRoutedToAutoAttendant(
            boolean inboundCallsRoutedToAutoAttendant) {
        this.inboundCallsRoutedToAutoAttendant = inboundCallsRoutedToAutoAttendant;
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

}
