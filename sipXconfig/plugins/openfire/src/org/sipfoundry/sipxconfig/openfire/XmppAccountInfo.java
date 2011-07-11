/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.openfire;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.service.SipxImbotService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

import static org.apache.commons.lang.StringUtils.defaultString;

public class XmppAccountInfo extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/xmpp-account-info-00-00";
    private static final String MUC_SUBDOMAIN = "conference";
    private static final String USER = "user";
    private static final String USER_NAME = "user-name";
    private static final String PASSWORD = "password";
    private static final String DESCRIPTION = "description";
    private static final String DISPLAY_NAME = "display-name";
    private static final String SIP_USER_NAME = "sip-user-name";
    private static final String EMAIL = "email";
    private static final String ON_THE_PHONE = "on-the-phone-message";
    private static final String ADVERTISE_ON_CALL = "advertise-on-call-status";
    private static final String SHOW_ON_CALL_DETAILS = "show-on-call-details";
    private static final String QUERY = "SELECT u.user_id, u.first_name, u.last_name, u.user_name, abe.im_id, "
            + "abe.im_password, abe.email_address, abe.im_display_name, "
            + "sv.value as on_the_phone, sp.value as sip_presence, si.value as call_info, v.value as im_enabled, "
            + "(SELECT count(*) from group_storage gs inner join setting_value sv on gs.group_id = sv.value_storage_id "
            + "inner join user_group ug on gs.group_id = ug.group_id where gs.resource='user' "
            + "AND ug.user_id=u.user_id AND sv.path='im/im-account' AND sv.value='1') as group_im_enabled "
            + "FROM users u left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "AND v.path='im/im-account' left join address_book_entry abe "
            + "on abe.address_book_entry_id = u.address_book_entry_id "
            + "left join setting_value sv on u.value_storage_id = sv.value_storage_id "
            + "AND sv.path='im/on-the-phone-message' "
            + "left join setting_value sp on u.value_storage_id = sp.value_storage_id "
            + "AND sp.path='im/advertise-sip-presence' "
            + "left join setting_value si on u.value_storage_id = si.value_storage_id "
            + "AND si.path='im/include-call-info' " + "WHERE u.user_type='C' ORDER BY u.user_id;";
    private static final String ENABLED = "1";
    private static final String DISABLED = "0";
    private CoreContext m_coreContext;
    private ConferenceBridgeContext m_conferenceContext;
    private SipxServiceManager m_sipxServiceManager;
    private JdbcTemplate m_jdbcTemplate;

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceContext) {
        m_conferenceContext = conferenceContext;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        final Element accountInfos = document.addElement("xmpp-account-info", NAMESPACE);

        // user to retrieve default setting values
        User userModel = m_coreContext.newUser();
        final String onThePhoneDefault = userModel.getSettings().getSetting("im/on-the-phone-message")
                .getDefaultValue();
        final String sipPresenceDefault = userModel.getSettings().getSetting("im/advertise-sip-presence")
                .getDefaultValue();
        final String callInfoDefault = userModel.getSettings().getSetting("im/include-call-info").getDefaultValue();
        m_jdbcTemplate.query(QUERY, new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                String enabled = StringUtils.defaultIfEmpty(rs.getString("im_enabled"), StringUtils.EMPTY);
                int groupsWithIm = rs.getInt("group_im_enabled");
                boolean imEnabled = enabled.equals(ENABLED) || (!enabled.equals(DISABLED) && groupsWithIm > 0);
                if (imEnabled) {
                    String onThePhone = StringUtils.defaultIfEmpty(rs.getString("on_the_phone"), onThePhoneDefault);
                    String sipPresence = StringUtils.defaultIfEmpty(rs.getString("sip_presence"), sipPresenceDefault);
                    String callInfo = StringUtils.defaultIfEmpty(rs.getString("call_info"), callInfoDefault);
                    boolean sipPresenceEnabled = false;
                    if (StringUtils.equals(sipPresence, ENABLED)) {
                        sipPresenceEnabled = true;
                    }
                    boolean callInfoEnabled = false;
                    if (StringUtils.equals(callInfo, ENABLED)) {
                        callInfoEnabled = true;
                    }
                    final User user = new User();
                    user.setUserName(rs.getString("user_name"));
                    user.setFirstName(rs.getString("first_name"));
                    user.setLastName(rs.getString("last_name"));
                    AddressBookEntry abe = new AddressBookEntry();
                    abe.setImId(rs.getString("im_id"));
                    abe.setImPassword(rs.getString("im_password"));
                    abe.setEmailAddress(rs.getString("email_address"));
                    abe.setImDisplayName(rs.getString("im_display_name"));
                    user.setAddressBookEntry(abe);
                    m_jdbcTemplate.query("select alias from user_alias where user_id="
                            + rs.getString("user_id") + ";", new RowCallbackHandler() {
                                @Override
                                public void processRow(ResultSet rs) throws SQLException {
                                    user.addAlias(rs.getString("alias"));
                                }
                            });
                    createUserAccount(user, onThePhone, sipPresenceEnabled, callInfoEnabled, accountInfos);
                }
            }
        });

        createPaUserAccount(accountInfos);

        List<Group> groups = m_coreContext.getGroups();
        for (Group group : groups) {
            createXmmpGroup(group, accountInfos);
        }

        List<Conference> conferences = m_conferenceContext.getAllConferences();
        for (Conference conf : conferences) {
            createXmppChatRoom(conf, accountInfos);
        }
        return document;
    }

    private void createPaUserAccount(Element accountInfos) {
        SipxImbotService imbotService = (SipxImbotService) m_sipxServiceManager
                .getServiceByBeanId(SipxImbotService.BEAN_ID);
        String paUserName = imbotService.getPersonalAssistantImId();
        String paPassword = imbotService.getPersonalAssistantImPassword();

        User paUser = m_coreContext.newUser();
        paUser.setUserName(paUserName);
        paUser.setImPassword(paPassword);
        ImAccount imAccount = new ImAccount(paUser);
        imAccount.setEnabled(true);

        createPaUserAccount(paUser, accountInfos);
    }

    private void createUserAccount(User user, String onThePhone, boolean sipPresence, boolean callInfo,
            Element accountInfos) {
        ImAccount imAccount = new ImAccount(user);

        Element userAccounts = accountInfos.addElement(USER);
        userAccounts.addElement(USER_NAME).setText(imAccount.getImId());
        userAccounts.addElement(SIP_USER_NAME).setText(user.getName());
        userAccounts.addElement(DISPLAY_NAME).setText(imAccount.getImDisplayName());
        userAccounts.addElement(PASSWORD).setText(imAccount.getImPassword());
        String email = imAccount.getEmailAddress();
        userAccounts.addElement(EMAIL).setText(email != null ? email : StringUtils.EMPTY);
        userAccounts.addElement(ON_THE_PHONE).setText(onThePhone);
        userAccounts.addElement(ADVERTISE_ON_CALL).setText(Boolean.toString(sipPresence));
        userAccounts.addElement(SHOW_ON_CALL_DETAILS).setText(Boolean.toString(callInfo));
    }

    private void createPaUserAccount(User user, Element accountInfos) {
        ImAccount imAccount = new ImAccount(user);
        if (!imAccount.isEnabled()) {
            return;
        }

        Element userAccounts = accountInfos.addElement(USER);
        userAccounts.addElement(USER_NAME).setText(imAccount.getImId());
        userAccounts.addElement(SIP_USER_NAME).setText(user.getName());
        userAccounts.addElement(DISPLAY_NAME).setText(imAccount.getImDisplayName());
        userAccounts.addElement(PASSWORD).setText(imAccount.getImPassword());
        String email = imAccount.getEmailAddress();
        userAccounts.addElement(EMAIL).setText(email != null ? email : StringUtils.EMPTY);
        userAccounts.addElement(ON_THE_PHONE).setText(imAccount.getOnThePhoneMessage());
        userAccounts.addElement(ADVERTISE_ON_CALL).setText(
                Boolean.toString(imAccount.advertiseSipPresence()));
        userAccounts.addElement(SHOW_ON_CALL_DETAILS).setText(Boolean.toString(imAccount.includeCallInfo()));
    }

    private void createXmppChatRoom(Conference conference, Element accountInfos) {
        if (!conference.isEnabled()) {
            return;
        }
        User owner = conference.getOwner();
        if (owner == null) {
            return;
        }
        ImAccount imAccount = new ImAccount(owner);
        if (!imAccount.isEnabled()) {
            return;
        }

        Element chatRoom = accountInfos.addElement("chat-room");

        String displayName = conference.getName();

        chatRoom.addElement("subdomain").setText(MUC_SUBDOMAIN);
        chatRoom.addElement("room-owner").setText(imAccount.getImId());
        chatRoom.addElement("room-name").setText(displayName);
        chatRoom.addElement(DESCRIPTION).setText(defaultString(conference.getDescription()));
        chatRoom.addElement(PASSWORD).setText(defaultString(conference.getParticipantAccessCode()));
        addBooleanSetting(chatRoom, conference, "moderated", "chat-meeting/moderated");
        addBooleanSetting(chatRoom, conference, "is-public-room", "chat-meeting/public");
        addBooleanSetting(chatRoom, conference, "is-members-only", "chat-meeting/members-only");
        chatRoom.addElement("is-persistent").setText(Boolean.TRUE.toString());
        chatRoom.addElement("conference-extension").setText(conference.getExtension());
    }

    private void addBooleanSetting(Element chatRoom, Conference conference, String elementName, String settingName) {
        Boolean value = (Boolean) conference.getSettingTypedValue(settingName);
        chatRoom.addElement(elementName).setText(value.toString());
    }

    private void createXmmpGroup(Group group, Element accountInfos) {
        // HACK: we assume that 'replicate-group' is a standard boolean setting
        Boolean replicate = (Boolean) group.getSettingTypedValue(new BooleanSetting(), "im/im-group");
        if (replicate == null || !replicate) {
            return;
        }

        Element xmmpGroup = accountInfos.addElement("group");
        xmmpGroup.addElement("group-name").setText(group.getName());
        String groupDescription = group.getDescription();
        if (groupDescription != null) {
            xmmpGroup.addElement(DESCRIPTION).setText(groupDescription);
        }

        Collection<User> groupMembers = m_coreContext.getGroupMembers(group);
        for (User user : groupMembers) {
            ImAccount imAccount = new ImAccount(user);
            if (imAccount.isEnabled()) {
                Element userElement = xmmpGroup.addElement(USER);
                userElement.addElement(USER_NAME).setText(imAccount.getImId());
            }
        }

        Boolean addPersonalAssistant = (Boolean) group.getSettingTypedValue(new BooleanSetting(),
                "im/add-pa-to-group");
        if (addPersonalAssistant != null && addPersonalAssistant) {
            Element userElement = xmmpGroup.addElement(USER);
            SipxImbotService imbotService = (SipxImbotService) m_sipxServiceManager
                    .getServiceByBeanId(SipxImbotService.BEAN_ID);
            String paUserName = imbotService.getPersonalAssistantImId();
            userElement.addElement(USER_NAME).setText(paUserName);
        }
    }

    public void setConfigJdbcTemplate(JdbcTemplate template) {
        m_jdbcTemplate = template;
    }
}
