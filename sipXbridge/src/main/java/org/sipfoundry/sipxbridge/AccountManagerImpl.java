/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import gov.nist.javax.sip.clientauthutils.UserCredentials;

import java.net.InetAddress;
import java.util.Collection;
import java.util.Hashtable;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import javax.sip.ClientTransaction;
import javax.sip.address.SipURI;

import org.apache.log4j.Logger;

/**
 * Keeps a mapping of account ID to ItspAccountInfo and a mapping of account ID to sip pbx account
 * info.
 * 
 * @author M. Ranganathan
 * 
 */
public class AccountManagerImpl implements gov.nist.javax.sip.clientauthutils.AccountManager {

    private static Logger logger = Logger.getLogger(AccountManagerImpl.class);

    private Hashtable<String, ItspAccountInfo> itspAccounts = new Hashtable<String, ItspAccountInfo>();

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
        try {
            for (ItspAccountInfo accountInfo : this.getItspAccounts()) {
                accountInfo.lookupAccount();
                this.addressToDomainNameMap.put(InetAddress.getByName(
                        accountInfo.getOutboundProxy()).getHostAddress(), accountInfo
                        .getSipDomain());

                this.domainNameToProxyAddressMap.put(accountInfo.getSipDomain(), new HopImpl(
                        InetAddress.getByName(accountInfo.getOutboundProxy()).getHostAddress(),
                        accountInfo.getProxyPort(), accountInfo.getOutboundTransport(),
                        accountInfo));
            }
        } catch (Exception ex) {
            throw new GatewayConfigurationException("Check configuration of ITSP Accounts ", ex);
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
        return itspAccounts.values().iterator().next();
    }

    /**
     * Get the outbound ITSP account for a specific outbund SipURI.
     */
    ItspAccountInfo getAccount(SipURI sipUri) {
        for (ItspAccountInfo accountInfo : itspAccounts.values()) {

            if (sipUri.getHost().endsWith(accountInfo.getProxyDomain())) {
                return accountInfo;
            }
        }
        return null;
    }

    /**
     * Get a collection of Itsp accounts.
     * 
     * @return
     */
    Collection<ItspAccountInfo> getItspAccounts() {

        return itspAccounts.values();
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
        int nport = port == -1 ? 5060 : port;

        for (HopImpl hop : this.domainNameToProxyAddressMap.values()) {
            if (hop.getHost().equals(host) && hop.getPort() == nport)
                return hop.getItspAccountInfo();
        }
        return null;
    }

    /**
     * Find the account info corresponding to a request URI.
     * 
     * @param requestURI
     * @return
     */
    ItspAccountInfo getItspAccount(SipURI requestURI) {
        String host = requestURI.getHost();
        for (ItspAccountInfo accountInfo : this.getItspAccounts()) {
            if (host.endsWith(accountInfo.getSipDomain())) {
                return accountInfo;
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

        String key = accountInfo.getAuthenticationRealm();
        this.itspAccounts.put(key, accountInfo);

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
