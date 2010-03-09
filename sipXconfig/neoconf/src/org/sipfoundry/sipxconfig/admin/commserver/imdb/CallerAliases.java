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
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class CallerAliases extends DataSetGenerator {
    private GatewayContext m_gatewayContext;

    private String m_anonymousAlias;

    @Override
    protected void addItems(final List<Map<String, String>> items) {
        // FIXME: use only gateways that are used in dialplan...
        List<Gateway> gateways = m_gatewayContext.getGateways();
        final String sipDomain = getSipDomain();

        for (Gateway gateway : gateways) {
            final String gatewayAddr = gateway.getGatewayAddress();
            final String gatewayAddrWithLineID = gatewayAddr + ";sipxecs-lineid=" + gateway.getId().toString();
            // add default entry for the gateway
            final GatewayCallerAliasInfo gatewayInfo = gateway.getCallerAliasInfo();
            String callerAliasUri = getGatewayCallerAliasUri(sipDomain, gatewayInfo);
            addItem(items, gatewayAddrWithLineID, callerAliasUri);

            // only add user aliases is overwrite is not set
            if (gatewayInfo.isIgnoreUserInfo() || gatewayInfo.isEnableCallerId()) {
                continue;
            }

            Closure<User> closure = new Closure<User>() {
                @Override
                public void execute(User user) {
                    String userCallerAliasUri = getCallerAliasUri(gatewayInfo, user);
                    String identity = AliasMapping.createUri(user.getUserName(), sipDomain);
                    addItem(items, gatewayAddrWithLineID, userCallerAliasUri, identity);
                }

            };
            forAllUsersDo(getCoreContext(), closure);
        }
    }

    private String getGatewayCallerAliasUri(String sipDomain, GatewayCallerAliasInfo gatewayInfo) {
        if (gatewayInfo.isAnonymous()) {
            return m_anonymousAlias;
        }
        String callerId = gatewayInfo.getCallerId();
        if (gatewayInfo.isEnableCallerId() && callerId != null) {
            return SipUri.fixWithDisplayName(callerId, gatewayInfo.getDisplayName(), gatewayInfo.getUrlParameters(),
                    sipDomain);
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

    @Override
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
