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

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.GatewayCallerAliasInfo;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.jdbc.core.RowCallbackHandler;

public class CallerAliases extends DataSetGenerator {
    private static final String QUERY = "SELECT u.user_id, u.first_name, u.last_name, u.user_name, "
            + "v.value as anonymous, sv.value as external_number, "
            + "(SELECT count(*) from user_group where user_id = u.user_id) as groups, "
            + "(SELECT count(*) from user_alias where user_id = u.user_id) as aliases FROM users u "
            + "left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "AND v.path='caller-alias/anonymous-caller-alias' "
            + "left join setting_value sv on u.value_storage_id = sv.value_storage_id "
            + "AND sv.path='caller-alias/external-number' WHERE u.user_type='C' ORDER BY u.user_id;";
    private static final String ALIAS = "alias";
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
            final User user = getCoreContext().newUser();
            List<Group> groups = getCoreContext().getGroups();
            final Map<Integer, Group> groupsMap = new HashMap<Integer, Group>();
            for (Group group : groups) {
                groupsMap.put(group.getId(), group);
            }

            getJdbcTemplate().query(QUERY, new RowCallbackHandler() {

                @Override
                public void processRow(ResultSet rs) throws SQLException {
                    String userId = rs.getString("user_id");
                    String userName = rs.getString("user_name");
                    String firstName = rs.getString("first_name");
                    String lastName = rs.getString("last_name");
                    user.setUserName(userName);
                    user.setFirstName(firstName);
                    user.setLastName(lastName);

                    int groupsCount = rs.getInt("groups");
                    user.setGroups(new TreeSet<Group>());
                    if (groupsCount > 0) {
                        // add groups to this user model
                        getJdbcTemplate().query("SELECT u.group_id from user_group u inner join group_storage s "
                                + "on u.group_id = s.group_id WHERE user_id=" + userId
                                + " AND s.resource='user';", new RowCallbackHandler() {
                                    @Override
                                    public void processRow(ResultSet rs) throws SQLException {
                                        user.addGroup(groupsMap.get(rs.getInt("group_id")));
                                    }
                                });
                    }
                    int aliasesCount = rs.getInt("aliases");
                    if (!user.hasNumericUsername() && aliasesCount > 0) {
                        getJdbcTemplate().query("select alias from user_alias where user_id=" + userId + ";",
                                new RowCallbackHandler() {
                                    @Override
                                    public void processRow(ResultSet rs) throws SQLException {
                                        user.addAlias(rs.getString(ALIAS));
                                    }
                                });
                    }
                    String anonymous = StringUtils.defaultIfEmpty(rs.getString("anonymous"), user.getSettings()
                            .getSetting(UserCallerAliasInfo.ANONYMOUS_CALLER_ALIAS).getDefaultValue());
                    String externalNumber = StringUtils.defaultIfEmpty(rs.getString("external_number"), user
                            .getSettings().getSetting(UserCallerAliasInfo.EXTERNAL_NUMBER).getDefaultValue());
                    String userCallerAliasUri = getCallerAliasUri(gatewayInfo, anonymous.equals("1"),
                            externalNumber, user);
                    String identity = AliasMapping.createUri(userName, sipDomain);
                    addItem(items, gatewayAddrWithLineID, userCallerAliasUri, identity);
                }
            });

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

    private String getCallerAliasUri(GatewayCallerAliasInfo gatewayInfo, boolean isAnonymous,
            String definedExternalNumber, User user) {
        if (isAnonymous) {
            return m_anonymousAlias;
        }
        // try transforming in gateway
        String externalNumber = gatewayInfo.getTransformedNumber(user);
        if (externalNumber == null) {
            // get number defined by user
            externalNumber = definedExternalNumber;
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
        item.put(ALIAS, alias);
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
