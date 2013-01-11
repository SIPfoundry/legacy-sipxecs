/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

import static org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType.XMPP_SERVER;
import static org.sipfoundry.sipxconfig.speeddial.SpeedDial.getResourceListId;

public class ResourceLists extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/resource-lists-00-01";
    private static final String QUERY = "SELECT u.user_id, u.user_name, v.value as im_enabled, vs.value as subscribe, "
            + "(SELECT count(*) from group_storage gs inner join setting_value sv on gs.group_id = sv.value_storage_id "
            + "inner join user_group ug on gs.group_id = ug.group_id where gs.resource='user' "
            + "AND ug.user_id=u.user_id AND sv.path='im/im-account' AND sv.value='1') as group_im_enabled, "
            + "(SELECT count(*) from speeddial d where d.user_id = u.user_id) as speed_count "
            + "from Users u left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "AND v.path='im/im-account' left join setting_value vs on u.value_storage_id = vs.value_storage_id "
            + "AND vs.path='permission/application/subscribe-to-presence' WHERE u.user_type='C' ORDER BY u.user_id;";

    private static final String USER_NAME = "user_name";
    private static final String LABEL = "label";
    private static final String NUMBER = "number";
    private CoreContext m_coreContext;
    private JdbcTemplate m_jdbcTemplate;

    @Override
    public boolean isLocationDependent() {
        return false;
    }

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        final Element lists = document.addElement("lists", NAMESPACE);
        final String domainName = m_coreContext.getDomainName();
        Element imLists = null;
        boolean xmppDisabled = m_jdbcTemplate.queryForInt("SELECT count(*) from setting_value where "
                        + "path='settings/watcher-enabled' and value='false'") > 0;
        m_jdbcTemplate.query(QUERY, new RlsRowCallbackHandler(lists, domainName, imLists, xmppDisabled));
        return document;
    }

    private Element createResource(Element list, String uri, String name) {
        Element resource = list.addElement("resource");
        // Append "sipx-noroute=Voicemail" and "sipx-userforward=false"
        // URI parameters to the target URI to control how the proxy forwards
        // SUBSCRIBEs to the resource URI.
        resource.addAttribute("uri", uri + ";sipx-noroute=VoiceMail;sipx-userforward=false");
        addNameElement(resource, name);
        return resource;
    }

    Element createResourceForUser(Element list, Button button, String domainName) {
        String name = StringUtils.defaultIfEmpty(button.getLabel(), button.getNumber());
        return createResource(list, buildUri(button, domainName), name);
    }

    String buildUri(final Button button, final String domainName) {
        final String number = button.getNumber();
        final StringBuilder uri = new StringBuilder();
        if (SipUri.matches(number)) {
            uri.append(SipUri.normalize(number));
        } else {
            // not a URI - check if we have a user
            m_jdbcTemplate.query("SELECT u.user_name from Users u inner join user_alias ua "
                    + "on ua.user_id = u.user_id WHERE ua.alias='" + number + "' " + "AND u.user_type='C'",
                    new RowCallbackHandler() {
                        @Override
                        public void processRow(ResultSet rs) throws SQLException {
                            String userName = rs.getString(USER_NAME);
                            if (StringUtils.isNotEmpty(userName)) {
                                button.setNumber(userName);
                            }
                        }
                    });
            uri.append(SipUri.format(button.getNumber(), domainName, false));
        }
        return uri.toString();
    }

    private void addNameElement(Element parent, String name) {
        parent.addElement("name").setText(name);
    }

    private Element createResourceList(Element lists, String name, String full, String consolidated) {
        Element list = lists.addElement("list");
        list.addAttribute("user", full);
        list.addAttribute("user-cons", consolidated);
        addNameElement(list, name);
        return list;
    }

    private Element createResourceList(Element lists, String name) {
        return createResourceList(lists, name, getResourceListId(name, false), getResourceListId(name, true));
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setConfigJdbcTemplate(JdbcTemplate template) {
        m_jdbcTemplate = template;
    }

    private class RlsRowCallbackHandler implements RowCallbackHandler {
        private Element m_imList;
        private final Element m_lists;
        private final String m_domainName;
        private final boolean m_xmppDisabled;

        public RlsRowCallbackHandler(Element lists, String domainName, Element imList, boolean xmppDisabled) {
            m_lists = lists;
            m_domainName = domainName;
            m_imList = imList;
            m_xmppDisabled = xmppDisabled;
        }

        @Override
        public void processRow(ResultSet rs) throws SQLException {
            boolean imEnabled = StringUtils.equals(rs.getString("im_enabled"), "1");
            int groupImEnabled = rs.getInt("group_im_enabled");
            String userName = StringUtils.defaultString(rs.getString(USER_NAME), StringUtils.EMPTY);
            if ((imEnabled || groupImEnabled > 0) && !m_xmppDisabled) {
                if (m_imList == null) {
                    m_imList = createResourceList(m_lists, XMPP_SERVER.getUserName());
                }
                createResource(m_imList, SipUri.format(userName, m_domainName, false), userName);
            }
            // check BLF buttons for this user
            final List<Button> buttons = new LinkedList<Button>();
            String userId = rs.getString("user_id");
            int speedCount = rs.getInt("speed_count");
            if (speedCount > 0) {
                m_jdbcTemplate.query("SELECT label, number from speeddial_button sp "
                        + "inner join speeddial d on sp.speeddial_id = d.speeddial_id WHERE d.user_id =  " + userId
                        + " AND blf='t' ORDER BY position;", new RowCallbackHandler() {
                            @Override
                            public void processRow(ResultSet rs) throws SQLException {
                                Button button = new Button();
                                button.setLabel(rs.getString(LABEL));
                                button.setNumber(rs.getString(NUMBER));
                                buttons.add(button);
                            }
                        });
            }

            if (buttons.isEmpty()) {
                // check user groups
                m_jdbcTemplate.query("SELECT u.group_id, label, number, blf from speeddial_group_button sp "
                        + "inner join speeddial_group d on sp.speeddial_id = d.speeddial_id "
                        + "inner join user_group u on u.group_id = d.group_id " + "WHERE u.user_id = " + userId
                        + " AND blf='t' ORDER BY u.group_id, position;", new RowCallbackHandler() {
                            @Override
                            public void processRow(ResultSet rs) throws SQLException {
                                Button button = new Button();
                                button.setLabel(rs.getString(LABEL));
                                button.setNumber(rs.getString(NUMBER));
                                buttons.add(button);
                            }
                        });
            }

            Element list = null;
            for (Button button : buttons) {
                if (list == null) {
                    list = createResourceList(m_lists, userName, SpeedDial.getResourceListId(userName, false),
                            SpeedDial.getResourceListId(userName, true));
                }
                createResourceForUser(list, button, m_domainName);
            }

        }

    }
}
