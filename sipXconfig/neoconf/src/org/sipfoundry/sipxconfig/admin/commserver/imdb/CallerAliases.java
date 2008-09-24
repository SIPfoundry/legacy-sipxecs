/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

public class CallerAliases extends DataSetGenerator {
    private GatewayContext m_gatewayContext;

    private String m_anonymousAlias;

    protected void addItems(List<Map<String, String>> items) {
        // FIXME: use only gateways that are used in dialplan...
        List<Gateway> gateways = m_gatewayContext.getGateways();
        List<User> users = getCoreContext().loadUsers();
        String sipDomain = getSipDomain();

        for (Gateway gateway : gateways) {
            String gatewayAddr = gateway.getGatewayAddress();
            // add default entry for the gateway
            GatewayCallerAliasInfo gatewayInfo = gateway.getCallerAliasInfo();
            String callerAliasUri = getGatewayCallerAliasUri(sipDomain, gatewayInfo);
            addItem(items, gatewayAddr, callerAliasUri);

            // only add user aliases is overwrite is not set
            if (gatewayInfo.isIgnoreUserInfo()) {
                continue;
            }
            // add per-user entries
            for (User user : users) {
                String userCallerAliasUri = getCallerAliasUri(gatewayInfo, user);
                String identity = AliasMapping.createUri(user.getUserName(), sipDomain);
                addItem(items, gatewayAddr, userCallerAliasUri, identity);
            }
        }
    }

    private String getGatewayCallerAliasUri(String sipDomain, GatewayCallerAliasInfo gatewayInfo) {
        if (gatewayInfo.isAnonymous()) {
            return m_anonymousAlias;
        }
        String callerId = gatewayInfo.getCallerId();
        if (gatewayInfo.isEnableCallerId() && callerId != null) {
            return SipUri.fixWithDisplayName(callerId, gatewayInfo.getDisplayName(), gatewayInfo
                    .getUrlParameters(), sipDomain);
        }
        String defaultExternalNumber = gatewayInfo.getDefaultCallerAlias();
        if (defaultExternalNumber != null) {
            return SipUri.format(defaultExternalNumber, sipDomain, false);
        }
        return null;
    }

    private String getCallerAliasUri(GatewayCallerAliasInfo gatewayInfo, User user) {
        UserCallerAliasInfo info = new UserCallerAliasInfo(user);
        if (info.isAnonymous()) {
            return m_anonymousAlias;
        }
        // try transforming in gateway
        String externalNumber = gatewayInfo.getTransformedNumber(user);
        if (externalNumber == null) {
            // get number defined by user
            externalNumber = info.getExternalNumber();
        }
        if (externalNumber != null) {
            // if we found the number we can return it
            return SipUri.format(user.getDisplayName(), externalNumber, getSipDomain());
        }
        return null;
    }

    private Map<String, String> addItem(List<Map<String, String>> items, String domain, String alias, String identity) {
        if (StringUtils.isEmpty(alias)) {
            // nothing to add
            return null;
        }

        Map<String, String> item = addItem(items);
        if (identity != null) {
            item.put("identity", identity);
        }
        item.put("domain", domain);
        item.put("alias", alias);
        return item;
    }

    private Map<String, String> addItem(List<Map<String, String>> items, String domain, String alias) {
        return addItem(items, domain, alias, null);
    }

    protected DataSet getType() {
        return DataSet.CALLER_ALIAS;
    }

    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    public void setAnonymousAlias(String anonymousAlias) {
        m_anonymousAlias = anonymousAlias;
    }
}
