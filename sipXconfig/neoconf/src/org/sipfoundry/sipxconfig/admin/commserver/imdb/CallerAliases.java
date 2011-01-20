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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.mongodb.DBObject;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;

public class CallerAliases extends DataSetGenerator {
    public static final String CALLERALIASES = "cals";
    public static final String LINEID = ";sipxecs-lineid=";
    private GatewayContext m_gatewayContext;

    private String m_anonymousAlias;

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

    @Override
    public void generate() {
        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                generate(user);
            }

        };
        DaoUtils.forAllUsersDo(getCoreContext(), closure);
        // gw
        List<Gateway> gateways = m_gatewayContext.getGateways();
        for (Gateway gateway : gateways) {
            generate(gateway);
        }
    }

    @Override
    public void generate(Replicable entity) {
        if (entity instanceof User) {
            List<CallerAliasesMapping> mappings = new ArrayList<CallerAliasesMapping>();
            User user = (User) entity;
            // FIXME: use only gateways that are used in dialplan...
            List<Gateway> gateways = m_gatewayContext.getGateways();
            for (Gateway gateway : gateways) {
                final String gatewayAddr = gateway.getGatewayAddress();
                final String gatewayAddrWithLineID = gatewayAddr + LINEID + gateway.getId().toString();
                final GatewayCallerAliasInfo gatewayInfo = gateway.getCallerAliasInfo();
                // only add user aliases is overwrite is not set
                if (gatewayInfo.isIgnoreUserInfo() || gatewayInfo.isEnableCallerId()) {
                    continue;
                }
                String userCallerAliasUri = getCallerAliasUri(gatewayInfo, user);
                if (!StringUtils.isEmpty(userCallerAliasUri)) {
                    mappings.add(new CallerAliasesMapping(gatewayAddrWithLineID, userCallerAliasUri));
                }
            }
            insertCallerAliases(entity, mappings);
        } else if (entity instanceof Gateway) {
            Closure<User> closure = new Closure<User>() {
                @Override
                public void execute(User user) {
                    generate(user);
                }

            };
            DaoUtils.forAllUsersDo(getCoreContext(), closure);
            Gateway gateway = (Gateway) entity;
            final String gatewayAddr = gateway.getGatewayAddress();
            final String gatewayAddrWithLineID = gatewayAddr + LINEID + gateway.getId().toString();
            final GatewayCallerAliasInfo gatewayInfo = gateway.getCallerAliasInfo();
            String callerAliasUri = getGatewayCallerAliasUri(getSipDomain(), gatewayInfo);
            if (!StringUtils.isEmpty(callerAliasUri)) {
                insertCallerAliases(gateway,
                        Collections.singletonList(new CallerAliasesMapping(gatewayAddrWithLineID, callerAliasUri)));
            }
        }

    }

    private void insertCallerAliases(Replicable entity, List<CallerAliasesMapping> mappings) {
        DBObject top = findOrCreate(entity);
        top.put(CALLERALIASES, mappings);
        getDbCollection().save(top);
    }

}
