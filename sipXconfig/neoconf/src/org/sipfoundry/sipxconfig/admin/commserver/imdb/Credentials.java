/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.jdbc.core.RowCallbackHandler;

import static org.apache.commons.lang.StringUtils.defaultString;

public class Credentials extends DataSetGenerator {
    private static final String QUERY = "SELECT first_name, last_name, user_name, sip_password, pintoken "
            + "from users WHERE user_type='C' ORDER BY user_id;";
    private static final String CALL_GROUP_QUERY = "SELECT name, sip_password from call_group where enabled=true";
    private static final String PINTOKEN = "pintoken";
    private static final String SIP_PASSWORD = "sip_password";

    @Override
    protected DataSet getType() {
        return DataSet.CREDENTIAL;
    }

    @Override
    protected void addItems(final List<Map<String, String>> items) {
        final String domainName = getSipDomain();
        CoreContext coreContext = getCoreContext();
        final String realm = coreContext.getAuthorizationRealm();

        getJdbcTemplate().query(QUERY, new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String firstName = rs.getString("first_name");
                String lastName = rs.getString("last_name");
                String userName = rs.getString("user_name");
                String sipPassword = rs.getString(SIP_PASSWORD);
                String pintoken = rs.getString(PINTOKEN);
                Object[] names = {
                    firstName, lastName
                };
                String displayName = StringUtils.trimToNull(StringUtils.join(names, ' '));
                String contact = SipUri.format(displayName, userName, domainName);
                addCredentialsItem(items, contact, userName, sipPassword, pintoken, realm);
            }
        });

        List<InternalUser> internalUsers = getCoreContext().loadInternalUsers();
        for (InternalUser user : internalUsers) {
            addUser(items, user, domainName, realm);
        }

        for (SpecialUserType specialUserType : SpecialUserType.values()) {
            addSpecialUser(items, specialUserType, domainName, realm);
        }

        getJdbcTemplate().query(CALL_GROUP_QUERY, new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String name = rs.getString("name");
                String sipPassword = rs.getString(SIP_PASSWORD);
                addCallGroup(items, sipPassword, name, domainName, realm);
            }
        });
    }

    void addCallGroup(List<Map<String, String>> items, String sipPassword, String name, String domainName,
            String realm) {
        String uri = SipUri.format(null, name, domainName);
        String password = StringUtils.defaultString(sipPassword);
        String sipPasswordHash = Md5Encoder.digestPassword(name, realm, password);
        addCredentialsItem(items, uri, name, sipPassword, sipPasswordHash, realm);
    }

    private void addSpecialUser(List<Map<String, String>> items, SpecialUserType specialUserType, String domainName,
            String realm) {
        User user = getCoreContext().getSpecialUser(specialUserType);
        if (user != null) {
            addUser(items, user, domainName, realm);
        }
    }

    protected void addUser(List<Map<String, String>> items, AbstractUser user, String domainName, String realm) {
        String uri = user.getUri(domainName);
        addCredentialsItem(items, uri, user.getUserName(), user.getSipPassword(), user.getPintoken(), realm);
    }

    private void addCredentialsItem(List<Map<String, String>> items, String uri, String name, String sipPassword,
            String pintoken, String realm) {
        Map<String, String> item = addItem(items);
        item.put("uri", uri);
        item.put("realm", realm);
        item.put("userid", name);
        item.put("passtoken", defaultString(sipPassword));
        item.put(PINTOKEN, pintoken);
        item.put("authtype", "DIGEST");
    }

}
