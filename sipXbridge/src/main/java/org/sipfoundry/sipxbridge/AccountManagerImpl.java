/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.clientauthutils.UserCredentials;
import gov.nist.javax.sip.header.ims.PPreferredIdentityHeader;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Collection;
import java.util.HashSet;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import javax.sip.ClientTransaction;
import javax.sip.address.SipURI;
import javax.sip.header.FromHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;
import org.xbill.DNS.TextParseException;

/**
 * Keeps a mapping of account ID to ItspAccountInfo and a mapping of account ID to sip pbx account
 * info.
 * 
 * @author M. Ranganathan
 * 
 */
public class AccountManagerImpl implements gov.nist.javax.sip.clientauthutils.AccountManager {

    private static Logger logger = Logger.getLogger(AccountManagerImpl.class);

    private HashSet<ItspAccountInfo> itspAccounts = new HashSet<ItspAccountInfo>();

    /*
     * A concurrent hash map is need here because of dynamic updates of these records. It is read
     * by the AddressResolver of the JAIN-SIP stack.
     */
    private ConcurrentMap<String, HopImpl> domainNameToProxyAddressMap = new ConcurrentHashMap<String, HopImpl>();

    /*
     * The reverse name lookup map
     */
    private ConcurrentMap<String, String> addressToDomainNameMap = new ConcurrentHashMap<String, String>();

    private BridgeConfiguration bridgeConfiguration;

    public AccountManagerImpl() {

    }

    // //////////////////////////////////////////////////////////////////
    // Package local methods.
    // //////////////////////////////////////////////////////////////////
    /**
     * @return the bridgeConfiguration
     */
    BridgeConfiguration getBridgeConfiguration() {
        return bridgeConfiguration;
    }

    void lookupItspAccountAddresses() throws GatewayConfigurationException {

        for (ItspAccountInfo accountInfo : this.getItspAccounts()) {
          
            try {
                accountInfo.lookupAccount();
                this.addressToDomainNameMap.put(InetAddress.getByName(
                        accountInfo.getOutboundProxy()).getHostAddress(), accountInfo
                        .getSipDomain());

                this.domainNameToProxyAddressMap.put(accountInfo.getSipDomain(), new HopImpl(
                        InetAddress.getByName(accountInfo.getOutboundProxy()).getHostAddress(),
                        accountInfo.getOutboundProxyPort(), accountInfo.getOutboundTransport(),
                        accountInfo));
            } catch (TextParseException ex) {
                accountInfo.setState(AccountState.INVALID);
                logger.error("Error looking up address " + accountInfo.getProxyDomain());
                logger.error("Error looking up gateway configuration" , ex);
            } catch (UnknownHostException ex) {
                accountInfo.setState(AccountState.INVALID);
                logger.error("Unknown host exception looking up name",ex);
            }
        }

    }

    /**
     * Start the failure timout timers for each of the accounts we manage.
     */
    void startAuthenticationFailureTimers() {

        for (ItspAccountInfo itspAccountInfo : this.getItspAccounts()) {
            itspAccountInfo.startFailureCounterScanner();
        }
    }

    /**
     * Update the mapping in our table ( after an srv rescan )
     */
    void setHopToItsp(String domain, HopImpl proxyHop) {
        this.addressToDomainNameMap.put(proxyHop.getHost(), domain);

        this.domainNameToProxyAddressMap.put(domain, proxyHop);
    }

    /**
     * Get the default outbound ITSP account for outbound calls.
     */
    ItspAccountInfo getDefaultAccount() {
        return itspAccounts.iterator().next();
    }

    /**
     * Get the outbound ITSP account for a specific outbund SipURI.
     */
    ItspAccountInfo getAccount(Request request) {

        SipURI sipUri = (SipURI) request.getRequestURI();

        for (ItspAccountInfo accountInfo : itspAccounts) {

            if (sipUri.getHost().endsWith(accountInfo.getProxyDomain())) {
                if (accountInfo.getCallerId() == null) {
                    return accountInfo;
                } else {
                    String callerId = accountInfo.getCallerId();
                    FromHeader fromHeader = (FromHeader) request.getHeader(FromHeader.NAME);

                    String userStr = ((SipURI) fromHeader.getAddress().getURI()).getUser();
                    String domainStr = ((SipURI) fromHeader.getAddress().getURI()).getHost();
                    if (userStr.equals("anonymous") && domainStr.equals("invalid")) {
                        PPreferredIdentityHeader pai = (PPreferredIdentityHeader) request
                                .getHeader(PPreferredIdentityHeader.NAME);
                        if (pai == null) {
                            logger.warn("Anonymous call without P-Preferred-Identity ");
                            // BUGBUG - this is really a mistake we should reject
                            // the call if the PAI header is missing
                            return accountInfo;
                        }
                        userStr = ((SipURI) pai.getAddress().getURI()).getUser();
                    }
                    if (callerId.startsWith(userStr)) {
                        return accountInfo;
                    }

                }
            }
        }
        // Fallback -- cannot find calling line id. 
        for (ItspAccountInfo accountInfo : itspAccounts) { 
            if (sipUri.getHost().endsWith(accountInfo.getProxyDomain())) return accountInfo;
        }
        return null;
    }

    /**
     * Get a collection of Itsp accounts.
     * 
     * @return
     */
    Collection<ItspAccountInfo> getItspAccounts() {

        return itspAccounts;
    }

    /**
     * Resolves hop to ITSP account.
     * 
     * @param host
     * @return hop to the itsp account.
     */

    HopImpl getHopToItsp(String host) {
        return this.domainNameToProxyAddressMap.get(host);
    }

    /**
     * Get an ITSP account based on the host and port of the indbound request.
     * 
     * @param host
     * @param port
     * @return
     */
    ItspAccountInfo getItspAccount(String host, int port) {
        logger.debug("INVITE received on " + host ) ;
        for ( ItspAccountInfo accountInfo : this.getItspAccounts() ) {
            if ( accountInfo.isRegisterOnInitialization() ) {
                // Account needs registration.
                String registrarHost = accountInfo.getOutboundRegistrar();
                logger.debug("registrarHost = " + registrarHost);
               
                if ( host.equals(registrarHost)) {
                    logger.debug("found account " );
                    return accountInfo;
                }
                
            }
        }
        return null;
    }

    // //////////////////////////////////////////////////////////////////
    // Public methods.
    // //////////////////////////////////////////////////////////////////
    /**
     * Add the Bridge config.
     */
    public void setBridgeConfiguration(BridgeConfiguration bridgeConfiguration) {
        this.bridgeConfiguration = bridgeConfiguration;
    }

    /**
     * Add an ITSP account to the account database ( method is accessed by the digester).
     */
    public void addItspAccount(ItspAccountInfo accountInfo) throws GatewayConfigurationException {
        this.itspAccounts.add(accountInfo);
    }

    /*
     * (non-Javadoc)
     * 
     * @see gov.nist.javax.sip.clientauthutils.AccountManager#getCredentials(javax.sip.ClientTransaction,
     *      java.lang.String)
     */
    public UserCredentials getCredentials(ClientTransaction ctx, String authRealm) {

        TransactionApplicationData tad = (TransactionApplicationData) ctx.getApplicationData();

        return tad.itspAccountInfo;

    }

}
