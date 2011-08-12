/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.springframework.jdbc.core.RowCallbackHandler;

public class UserStatic extends DataSetGenerator {

    private static final String QUERY = "SELECT user_name, v.value as ext_mwi "
            + "FROM users u left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "WHERE u.user_type='C' AND v.path='voicemail/mailbox/external-mwi' ORDER BY u.user_id;";

    @Override
    protected DataSet getType() {
        return DataSet.USER_STATIC;
    }

    @Override
    protected void addItems(final List<Map<String, String>> items) {
        final String domainName = getSipDomain();
        getJdbcTemplate().query(QUERY, new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String userName = rs.getString("user_name");
                String extMwi = rs.getString("ext_mwi");
                addUser(items, userName, domainName, extMwi);
            }
        });
    }

    protected void addUser(List<Map<String, String>> items, String userName, String domainName, String extMwi) {
        Map<String, String> item = addItem(items);
        String identity = userName + "@" + domainName;
        item.put("identity", identity);
        item.put("event", "message-summary");
        item.put("contact", SipUri.format(extMwi, domainName, false));
        item.put("from_uri", SipUri.format("IVR", domainName, false));
        item.put("to_uri", SipUri.format(userName, domainName, false));
        item.put("callid", "static-mwi-" + identity);
    }
}
