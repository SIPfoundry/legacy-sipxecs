/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.sql.ResultSet;
import java.sql.SQLException;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

public class PresenceRoutingConfiguration extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/presencerouting-config-00-00";

    private static final String QUERY = "SELECT user_name, v.value as vmondnd FROM users u "
            + "left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "AND v.path='im/fwd-vm-on-dnd' WHERE u.user_type='C' ORDER BY u.user_id;";
    private JdbcTemplate m_jdbcTemplate;

    @Override
    public boolean isLocationDependent() {
        return false;
    }

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        final Element userPrefs = document.addElement("presenceRoutingPrefs", NAMESPACE);
        m_jdbcTemplate.query(QUERY, new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String userName = rs.getString("user_name");
                String dnd = rs.getString("vmondnd");
                boolean vmondnd = false;
                if (StringUtils.equals(dnd, "1")) {
                    vmondnd = true;
                }
                generateUserPrefXml(userName, vmondnd, userPrefs);
            }
        });

        return document;
    }

    private void generateUserPrefXml(String userName, boolean isFwdOnDnd, Element userPrefs) {
        Element userElement = userPrefs.addElement("user");
        userElement.addElement("userName").setText(userName);
        userElement.addElement("vmOnDnd").setText(Boolean.toString(isFwdOnDnd));
    }

    public void setConfigJdbcTemplate(JdbcTemplate template) {
        m_jdbcTemplate = template;
    }
}
