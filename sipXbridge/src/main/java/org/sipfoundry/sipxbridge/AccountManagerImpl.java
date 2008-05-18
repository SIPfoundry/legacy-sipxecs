/*
 *  Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Collection;
import java.util.Hashtable;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;

import org.apache.log4j.Logger;
import org.xbill.DNS.*;

import javax.sip.address.SipURI;
import gov.nist.javax.sip.clientauthutils.*;

/**
 * Keeps a mapping of account ID to ItspAccountInfo and a mapping of account ID
 * to sip pbx account info.
 * 
 * @author M. Ranganathan
 * 
 */
public class AccountManagerImpl implements
        gov.nist.javax.sip.clientauthutils.AccountManager {

    private static Logger logger = Logger.getLogger(AccountManagerImpl.class);

    private Hashtable<String, ItspAccountInfo> itspAccounts = new Hashtable<String, ItspAccountInfo>();

    /*
     * A concurrent hash map is need here because of dynamic updates of these
     * records. It is read by the AddressResolver of the JAIN-SIP stack.
     */
    private ConcurrentMap<String, HopImpl> domainNameToProxyAddressMap = new ConcurrentHashMap<String, HopImpl>();

    /*
     * The reverse name lookup map
     */
    private ConcurrentMap<String, String> addressToDomainNameMap = new ConcurrentHashMap<String, String>();

    private BridgeConfiguration bridgeConfiguration;

    public AccountManagerImpl() {

    }

    /**
     * Add the Bridge config.
     */
    public void setBridgeConfiguration(BridgeConfiguration bridgeConfiguration) {
        this.bridgeConfiguration = bridgeConfiguration;
    }

    /**
     * @return the bridgeConfiguration
     */
    public BridgeConfiguration getBridgeConfiguration() {
        return bridgeConfiguration;
    }

    /**
     * Add an ITSP account to the databse.
     */
    public void addItspAccount(ItspAccountInfo accountInfo)
            throws GatewayConfigurationException {

        String key = accountInfo.getAuthenticationRealm();
        this.itspAccounts.put(key, accountInfo);

    }

    public void lookupItspAccountAddresses()
            throws GatewayConfigurationException {
        try {
            for (ItspAccountInfo accountInfo : this.getItspAccounts()) {
                accountInfo.lookupAccount();
                this.addressToDomainNameMap.put(InetAddress.getByName(
                        accountInfo.getOutboundProxy()).getHostAddress(),
                        accountInfo.getSipDomain());

                this.domainNameToProxyAddressMap.put(
                        accountInfo.getSipDomain(), new HopImpl(InetAddress
                                .getByName(accountInfo.getOutboundProxy())
                                .getHostAddress(), accountInfo.getProxyPort(),
                                accountInfo.getOutboundTransport(), accountInfo));
            }
        } catch (Exception ex) {
            throw new GatewayConfigurationException(
                    "Check configuration of ITSP Accounts ", ex);
        }
    }

    /**
     * Start the failure timout timers for each of the accounts we manage.
     */
    public void startAuthenticationFailureTimers() {

        for (ItspAccountInfo itspAccountInfo : this.getItspAccounts()) {
            itspAccountInfo.startFailureCounterScanner();
        }
    }

    /**
     * Update the mapping in our table ( after an srv rescan )
     */
    public void setHopToItsp(String domain, HopImpl proxyHop) {
        this.addressToDomainNameMap.put(proxyHop.getHost(), domain);

        this.domainNameToProxyAddressMap.put(domain, proxyHop);
    }

    /**
     * Get the default outbound ITSP account for outbound calls.
     */
    public ItspAccountInfo getDefaultAccount() {
        return itspAccounts.values().iterator().next();
    }

    /**
     * Get the outbound ITSP account for a specific outbund SipURI.
     */
    public ItspAccountInfo getAccount(SipURI sipUri) {
        for (ItspAccountInfo accountInfo : itspAccounts.values()) {
            
            if (sipUri.getHost().endsWith(accountInfo.getProxyDomain())) {
                return accountInfo;
            } 
        }
        return null;
    }

    /**
     * Get the user creds for a userName:domainName pair.
     * 
     * @param userName -
     *            the name of the user for whom we want the acct.
     * @param domainName -
     *            the domain name for which we want creds.
     * 
     */
    public UserCredentials getCredentials(SipURI sipUri, String authRealm) {

        UserCredentials retval = this.itspAccounts.get(authRealm);
        if (retval == null) {
            // Maybe he is coming in with an address instead of a domain name.
            // do a reverse lookup.
            String realDomainName = this.addressToDomainNameMap.get(authRealm);
            if (realDomainName == null) {
                // Check to see if we have any creds based upon his URI.
                String host = sipUri.getHost();
                retval = this.itspAccounts.get(host);
                
            

            } else {
                retval = this.itspAccounts.get(realDomainName);
            }
        }
        if ( retval == null ) {
            if (logger.isDebugEnabled()) {
                logger.debug("No creds for account " + sipUri + " realm " + authRealm);
            }
        }
        return retval;

    }

    /**
     * Get a collection of Itsp accounts.
     * 
     * @return
     */
    public Collection<ItspAccountInfo> getItspAccounts() {

        return itspAccounts.values();
    }

    /**
     * Resolves hop to ITSP account.
     * 
     * @param host
     * @return hop to the itsp account.
     */

    public HopImpl getHopToItsp(String host) {
        return this.domainNameToProxyAddressMap.get(host);
    }

    public ItspAccountInfo getItspAccount(String host, int port) {
        int nport = port == -1 ? 5060 : port;
        
        for ( HopImpl hop : this.domainNameToProxyAddressMap.values()) {
            if ( hop.getHost().equals(host) && hop.getPort() == nport ) return hop.getItspAccountInfo();
        }
        return null;
    }

}
