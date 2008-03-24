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
        logger.debug("adding ITSP Account");
        try {
            String key = accountInfo.getSipDomain();
            this.itspAccounts.put(key, accountInfo);
            String outboundDomain = accountInfo.getSipDomain();

            Record[] records = new Lookup("_sip._" + accountInfo.getTransport()
                    + "." + outboundDomain, Type.SRV).run();

            if (records == null || records.length == 0) {
                // SRV lookup failed, use the outbound proxy directly.
                logger
                        .debug("SRV lookup returned nothing -- we are going to just use the domain name directly");
            } else {
                logger.debug("Did a successful DNS SRV lookup");
                SRVRecord record = (SRVRecord) records[0];
                int port = record.getPort();
                accountInfo.setPort(port);
                long time = record.getTTL() * 1000;
                String resolvedName = record.getTarget().toString();
                accountInfo.setOutboundProxy(resolvedName);
                accountInfo.startDNSScannerThread(time);
            }

            this.addressToDomainNameMap.put(InetAddress.getByName(
                    accountInfo.getOutboundProxy()).getHostAddress(), key);

            this.domainNameToProxyAddressMap.put(key, new HopImpl(
                    InetAddress.getByName(accountInfo.getOutboundProxy())
                            .getHostAddress(), accountInfo.getProxyPort(),
                    accountInfo.getTransport()));
        } catch (TextParseException ex) {
            throw new GatewayConfigurationException(
                    "Problem with domain name lookup", ex);
        } catch (UnknownHostException e) {
            throw new GatewayConfigurationException("Problem with name lookup",
                    e);
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
            if (accountInfo.getProxyDomain().equals(sipUri.getHost())) {
                return accountInfo;
            }
        }
        return getDefaultAccount();
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
    public UserCredentials getCredentials(String domainName) {

        UserCredentials retval = this.itspAccounts.get(domainName);
        if (retval == null) {
            // Maybe he is coming in with an address instead of a domain name.
            // do a reverse lookup.
            String realDomainName = this.addressToDomainNameMap.get(domainName);
            if (realDomainName == null) {
                logger.debug("No creds found for " + domainName);

            } else {
                retval = this.itspAccounts.get(realDomainName);
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

   

}
