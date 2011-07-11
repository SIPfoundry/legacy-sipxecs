/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.lang.reflect.InvocationTargetException;
import java.sql.ResultSet;
import java.sql.SQLException;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.service.SipxImbotService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

public class ContactInformationConfig extends XmlFile {
    private static final Log LOG = LogFactory.getLog(ContactInformationConfig.class);

    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/contactinfo-00-00";
    private static final String QUERY = "SELECT u.user_id, u.user_name, u.first_name, u.last_name, "
            + "v.value as conf_entry, sv.value as conf_exit, sp.value as msg_begin, si.value as msg_end, "
            + "abe.im_id, abe.im_display_name, abe.alternate_im_id, abe.job_title, abe.job_dept, "
            + "abe.company_name, abe.assistant_name, abe.assistant_phone_number, "
            + "abe.fax_number, abe.location, abe.home_phone_number, abe.cell_phone_number, "
            + "ha.street as home_street, ha.city as home_city, ha.country as home_country, "
            + "ha.state as home_state, ha.zip as home_zip, ha.office_designation as home_office_designation, "
            + "ja.street as job_street, ja.zip as job_zip, "
            + "ja.city as job_city, ja.country as job_country, ja.state as job_state, "
            + "(SELECT count(*) from user_alias where user_id = u.user_id) as alias_count, "
            + "(SELECT count(*) from group_storage gs inner join setting_value sv on "
            + "gs.group_id = sv.value_storage_id inner join user_group ug "
            + "on gs.group_id = ug.group_id where gs.resource='user' AND "
            + "ug.user_id=u.user_id AND sv.path='im_notification/conferenceEntryIM' AND sv.value='0') "
            + "as conf_entry_disabled, "
            + "(SELECT count(*) from group_storage gs "
            + "inner join setting_value sv on "
            + "gs.group_id = sv.value_storage_id "
            + "inner join user_group ug on gs.group_id = ug.group_id "
            + "where gs.resource='user' "
            + "AND ug.user_id=u.user_id AND sv.path='im_notification/conferenceExitIM' AND sv.value='0') "
            + "as conf_exit_disabled, "
            + "ja.office_designation as job_office_designation, "
            + "(SELECT count(*) from group_storage gs inner join setting_value sv "
            + "on gs.group_id = sv.value_storage_id inner join user_group ug on "
            + "gs.group_id = ug.group_id where gs.resource='user' "
            + "AND ug.user_id=u.user_id AND sv.path='im_notification/leaveMsgBeginIM' AND sv.value='1') "
            + "as leave_msg_begin_enabled, "
            + "(SELECT count(*) from group_storage gs inner join setting_value sv on gs.group_id = sv.value_storage_id "
            + "inner join user_group ug on gs.group_id = ug.group_id where gs.resource='user' "
            + "AND ug.user_id=u.user_id AND sv.path='im_notification/leaveMsgEndIM' AND sv.value='1') "
            + "as leave_msg_end_enabled from Users u "
            + "left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "AND v.path='im_notification/conferenceEntryIM' "
            + "left join setting_value sv on u.value_storage_id = sv.value_storage_id "
            + "AND sv.path='im_notification/conferenceExitIM' "
            + "left join setting_value sp on u.value_storage_id = sp.value_storage_id "
            + "AND sp.path='im_notification/leaveMsgBeginIM' "
            + "left join setting_value si on u.value_storage_id = si.value_storage_id "
            + "AND si.path='im_notification/leaveMsgEndIM' "
            + "left join address_book_entry abe on abe.address_book_entry_id = u.address_book_entry_id "
            + "left join address ha on ha.address_id = abe.home_address_id "
            + "left join address ja on ja.address_id = abe.office_address_id "
            + "WHERE u.user_type='C' ORDER BY u.user_id;";
    private static final String STREET = "street";
    private static final String CITY = "city";
    private static final String COUNTRY = "country";
    private static final String STATE = "state";
    private static final String ZIP = "zip";
    private static final String OFFICE_DESIGNATION = "officeDesignation";
    private static final String LOCATION = "location";
    private static final String NAME = "name";
    private static final String EXTENSION = "extension";
    private static final String PIN = "pin";
    private static final String SQL_END = ";";
    private static final String DISABLED = "0";
    private static final String ENABLED = "1";
    private SipxServiceManager m_sipxServiceManager;
    private JdbcTemplate m_jdbcTemplate;
    private CoreContext m_coreContext;

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        final Element contactInfos = document.addElement("contact-info", NAMESPACE);

        // user to retrieve default setting values
        User userModel = m_coreContext.newUser();
        final String confEntryDefault = userModel.getSettings().getSetting("im_notification/conferenceEntryIM")
                .getDefaultValue();
        final String confExitDefault = userModel.getSettings().getSetting("im_notification/conferenceExitIM")
                .getDefaultValue();
        final String leaveMsgBeginDefault = userModel.getSettings().getSetting("im_notification/leaveMsgBeginIM")
                .getDefaultValue();
        final String leaveMsgEndDefault = userModel.getSettings().getSetting("im_notification/leaveMsgEndIM")
                .getDefaultValue();
        m_jdbcTemplate.query(QUERY, new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String userId = rs.getString("user_id");
                String userName = rs.getString("user_name");
                String firstName = rs.getString("first_name");
                String lastName = rs.getString("last_name");
                final User user = new User();
                user.setUserName(userName);
                user.setFirstName(firstName);
                user.setLastName(lastName);
                AddressBookEntry abe = new AddressBookEntry();
                abe.setImId(rs.getString("im_id"));
                abe.setImDisplayName(rs.getString("im_display_name"));
                user.setAddressBookEntry(abe);

                int aliasCount = rs.getInt("alias_count");
                if (aliasCount > 0) {
                    m_jdbcTemplate.query("select alias from user_alias where user_id=" + userId + SQL_END,
                            new RowCallbackHandler() {
                                @Override
                                public void processRow(ResultSet rs) throws SQLException {
                                    user.addAlias(rs.getString("alias"));
                                }
                            });
                }
                Element userEl = contactInfos.addElement("user");
                userEl.addElement("userName").setText(user.getUserName());

                ImAccount imAccount = new ImAccount(user);
                addElements(userEl, imAccount, "imId", "imDisplayName");
                addElement(userEl, rs.getString("alternate_im_id"), "alternateImId");
                addElement(userEl, rs.getString("job_title"), "jobTitle");
                addElement(userEl, rs.getString("job_dept"), "jobDept");
                addElement(userEl, rs.getString("company_name"), "companyName");
                addElement(userEl, rs.getString("assistant_name"), "assistantName");
                addElement(userEl, rs.getString("assistant_phone_number"), "assistantPhoneNumber");
                addElement(userEl, rs.getString("fax_number"), "faxNumber");
                addElement(userEl, rs.getString(LOCATION), LOCATION);
                addElement(userEl, rs.getString("home_phone_number"), "homePhoneNumber");
                addElement(userEl, rs.getString("cell_phone_number"), "cellPhoneNumber");

                // home address
                Element homeAddressEl = userEl.addElement("homeAddress");
                addElement(homeAddressEl, rs.getString("home_street"), STREET);
                addElement(homeAddressEl, rs.getString("home_city"), CITY);
                addElement(homeAddressEl, rs.getString("home_country"), COUNTRY);
                addElement(homeAddressEl, rs.getString("home_state"), STATE);
                addElement(homeAddressEl, rs.getString("home_zip"), ZIP);
                addElement(homeAddressEl, rs.getString("home_office_designation"), OFFICE_DESIGNATION);

                // office address
                Element officeAddressEl = userEl.addElement("officeAddress");
                addElement(officeAddressEl, rs.getString("job_street"), STREET);
                addElement(officeAddressEl, rs.getString("job_city"), CITY);
                addElement(officeAddressEl, rs.getString("job_country"), COUNTRY);
                addElement(officeAddressEl, rs.getString("job_state"), STATE);
                addElement(officeAddressEl, rs.getString("job_zip"), ZIP);
                addElement(officeAddressEl, rs.getString("job_office_designation"), OFFICE_DESIGNATION);

                // add conferences
                final Element conferencesEl = userEl.addElement("conferences");
                m_jdbcTemplate.query("SELECT name, extension, v.value as pin from meetme_conference mc "
                        + "left join setting_value v on v.value_storage_id = mc.value_storage_id "
                        + "AND v.path='fs-conf-conference/participant-code' where owner_id = " + userId + SQL_END,
                        new RowCallbackHandler() {
                            @Override
                            public void processRow(ResultSet rs) throws SQLException {
                                Element conferenceElement = conferencesEl.addElement("conference");
                                conferenceElement.addElement(NAME).setText(rs.getString(NAME));
                                conferenceElement.addElement(EXTENSION).setText(rs.getString(EXTENSION));
                                String pin = rs.getString(PIN);
                                if (StringUtils.isNotEmpty(pin)) {
                                    conferenceElement.addElement(PIN).setText(pin);
                                }
                            }
                        });

                String confEntry = rs.getString("conf_entry");
                if (rs.getInt("conf_entry_disabled") > 0) {
                    confEntry = StringUtils.defaultIfEmpty(confEntry, DISABLED);
                } else {
                    confEntry = StringUtils.defaultIfEmpty(confEntry, confEntryDefault);
                }
                userEl.addElement("conferenceEntryIM").setText(confEntry);

                String confExit = rs.getString("conf_exit");
                if (rs.getInt("conf_exit_disabled") > 0) {
                    confExit = StringUtils.defaultIfEmpty(confExit, DISABLED);
                } else {
                    confExit = StringUtils.defaultIfEmpty(confExit, confExitDefault);
                }
                userEl.addElement("conferenceExitIM").setText(confExit);

                String leaveMsgBegin = rs.getString("msg_begin");
                if (rs.getInt("leave_msg_begin_enabled") > 0) {
                    leaveMsgBegin = StringUtils.defaultIfEmpty(leaveMsgBegin, ENABLED);
                } else {
                    leaveMsgBegin = StringUtils.defaultIfEmpty(leaveMsgBegin, leaveMsgBeginDefault);
                }
                userEl.addElement("leaveMsgBeginIM").setText(leaveMsgBegin);

                String leaveMsgEnd = rs.getString("msg_end");
                if (rs.getInt("leave_msg_end_enabled") > 0) {
                    leaveMsgEnd = StringUtils.defaultIfEmpty(leaveMsgEnd, ENABLED);
                } else {
                    leaveMsgEnd = StringUtils.defaultIfEmpty(leaveMsgEnd, leaveMsgEndDefault);
                }
                userEl.addElement("leaveMsgEndIM").setText(leaveMsgEnd);
            }
        });

        return document;
    }

    private void addElement(Element element, String value, String name) {
        if (StringUtils.isNotEmpty(value)) {
            element.addElement(name).setText(value);
        }
    }

    private void addElement(Element userEl, Object bean, String name) {
        try {
            String value = BeanUtils.getSimpleProperty(bean, name);
            if (!StringUtils.isEmpty(value)) {
                userEl.addElement(name).setText(value);
            }
        } catch (IllegalAccessException e) {
            LOG.error(e);
        } catch (InvocationTargetException e) {
            LOG.error(e);
        } catch (NoSuchMethodException e) {
            LOG.error(e);
        }
    }

    private void addElements(Element userEl, Object bean, String... names) {
        for (String name : names) {
            addElement(userEl, bean, name);
        }
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Override
    public boolean isReplicable(Location location) {
        return m_sipxServiceManager.isServiceInstalled(location.getId(), SipxImbotService.BEAN_ID)
                && m_sipxServiceManager.getServiceByBeanId(SipxImbotService.BEAN_ID).isAvailable();
    }

    public void setConfigJdbcTemplate(JdbcTemplate template) {
        m_jdbcTemplate = template;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
