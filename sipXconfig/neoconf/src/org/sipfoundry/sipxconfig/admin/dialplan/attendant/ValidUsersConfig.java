/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.StringUtils;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.QName;
import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManagerImpl;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.ActiveGreeting;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.AttachType;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.MailFormat;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.VoicemailTuiType;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.jdbc.core.RowCallbackHandler;

import static org.apache.commons.lang.StringUtils.EMPTY;
import static org.apache.commons.lang.StringUtils.defaultIfEmpty;
import static org.apache.commons.lang.StringUtils.defaultString;

public class ValidUsersConfig extends XmlFile {
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/validusers-00-00";

    private static final String ELEMENT_NAME_ALIAS = "alias";
    private static final String ELEMENT_NAME_ALIASES = "aliases";
    private static final String ELEMENT_NAME_CONTACT = "contact";
    private static final String ELEMENT_NAME_DISPLAYNAME = "displayName";
    private static final String ELEMENT_NAME_IDENTITY = "identity";
    private static final String ELEMENT_NAME_INDIRECTORY = "inDirectory";
    private static final String ELEMENT_NAME_PINTOKEN = "pintoken";
    private static final String ELEMENT_NAME_PASSTOKEN = "passtoken";
    private static final String ELEMENT_NAME_USER = "user";
    private static final String ELEMENT_NAME_USERNAME = "userName";
    private static final String ELEMENT_NAME_HASVOICEMAIL = "hasVoicemail";
    private static final String ELEMENT_NAME_USERBUSYPROMPT = "userBusyPrompt";
    private static final String ELEMENT_NAME_VOICEMAILTUI = "voicemailTui";
    private static final String ELEMENT_NAME_CANRECORDPROMPTS = "canRecordPrompts";
    private static final String ELEMENT_NAME_CANTUICHANGEPIN = "canTuiChangePin";
    private static final String ELEMENT_NAME_EMAIL = "email";
    private static final String ELEMENT_NAME_ADDRESS = "address";
    private static final String ELEMENT_NAME_NOTIFICATION = "notification";
    private static final String ELEMENT_NAME_ATTACH_AUDIO = "attachAudio";
    private static final String ELEMENT_NAME_IMAP = "imap";
    private static final String ELEMENT_NAME_SYNC = "synchronize";
    private static final String ELEMENT_NAME_HOST = "host";
    private static final String ELEMENT_NAME_PORT = "port";
    private static final String ELEMENT_NAME_TLS = "useTLS";
    private static final String ELEMENT_NAME_ACCOUNT = "account";
    private static final String ELEMENT_NAME_PASSWD = "password";
    private static final String ELEMENT_NAME_MOH = "moh";
    private static final String QUERY = "SELECT u.user_id, u.user_name, u.first_name, u.last_name, "
            + "u.pintoken, u.sip_password, v.value as in_directory, vs.value as has_voicemail, "
            + "vb.value as user_busy_prompt, vt.value as voicemail_tui, sr.value as record_prompts, "
            + "vp.value as tui_change_pin, abe.email_address, abe.alternate_email_address, "
            + "ag.value as active_greeting, em.value as external_mwi, "
            + "pe.value as primary_email_notif, pf.value as email_format, af.value as alt_email_format, "
            + "an.value as alt_email_notif, ih.value as imap_host, ip.value as imap_port, it.value as imap_tls, "
            + "ips.value as imap_password, iac.value as imap_account, pea.value as primary_email_attach, "
            + "aea.value as alt_email_attach, mh.value as user_moh, "
            + "(SELECT count(*) from user_group where user_id = u.user_id) as groups, "
            + "(SELECT count(*) from user_alias where user_id = u.user_id) as alias_count "
            + "from Users u left join setting_value v on u.value_storage_id = v.value_storage_id "
            + "AND v.path='permission/call-handling/AutoAttendant' "
            + "left join setting_value vs on u.value_storage_id = vs.value_storage_id "
            + "AND vs.path='permission/call-handling/Voicemail' "
            + "left join setting_value vb on u.value_storage_id = vb.value_storage_id "
            + "AND vb.path='voicemail/mailbox/user-busy-prompt' "
            + "left join setting_value vt on u.value_storage_id = vt.value_storage_id "
            + "AND vt.path='voicemail/mailbox/voicemail-tui' "
            + "left join setting_value sr on u.value_storage_id = sr.value_storage_id "
            + "AND sr.path='permission/call-handling/RecordSystemPrompts' "
            + "left join setting_value vp on u.value_storage_id = vp.value_storage_id "
            + "AND vp.path='permission/application/tui-change-pin' "
            + "left join setting_value ag on u.value_storage_id = ag.value_storage_id "
            + "AND ag.path='voicemail/mailbox/active-greeting' "
            + "left join setting_value em on u.value_storage_id = em.value_storage_id "
            + "AND em.path='voicemail/mailbox/external-mwi' "
            + "left join setting_value pe on u.value_storage_id = pe.value_storage_id "
            + "AND pe.path='voicemail/mailbox/primary-email-voicemail-notification' "
            + "left join setting_value pf on u.value_storage_id = pf.value_storage_id "
            + "AND pf.path='voicemail/mailbox/primary-email-format' "
            + "left join setting_value af on u.value_storage_id = af.value_storage_id "
            + "AND af.path='voicemail/mailbox/alternate-email-format' "
            + "left join setting_value an on u.value_storage_id = an.value_storage_id "
            + "AND an.path='voicemail/mailbox/alternate-email-voicemail-notification' "
            + "left join setting_value ih on u.value_storage_id = ih.value_storage_id "
            + "AND ih.path='voicemail/imap/host' "
            + "left join setting_value ip on u.value_storage_id = ip.value_storage_id "
            + "AND ip.path='voicemail/imap/port' "
            + "left join setting_value it on u.value_storage_id = it.value_storage_id "
            + "AND it.path='voicemail/imap/tls' "
            + "left join setting_value ips on u.value_storage_id = ips.value_storage_id "
            + "AND ips.path='voicemail/imap/password' "
            + "left join setting_value iac on u.value_storage_id = iac.value_storage_id "
            + "AND iac.path='voicemail/imap/account' "
            + "left join setting_value pea on u.value_storage_id = pea.value_storage_id "
            + "AND pea.path='voicemail/mailbox/primary-email-attach-audio' "
            + "left join setting_value aea on u.value_storage_id = aea.value_storage_id "
            + "AND aea.path='voicemail/mailbox/alternate-email-attach-audio' "
            + "left join setting_value mh on u.value_storage_id = mh.value_storage_id "
            + "AND mh.path='moh/audio-source' "
            + "left join address_book_entry abe on abe.address_book_entry_id = u.address_book_entry_id "
            + "WHERE u.user_type='C' ORDER BY u.user_id;";
    private JdbcTemplate m_jdbcTemplate;

    private DomainManager m_domainManager;

    private AliasProvider m_aliasProvider;

    private CoreContext m_coreContext;

    @Override
    public boolean isLocationDependent() {
        return false;
    }

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        QName validUsersName = FACTORY.createQName("validusers", NAMESPACE);
        final Element usersEl = document.addElement(validUsersName);
        final String domainName = m_domainManager.getDomain().getName();
        final String realm = m_domainManager.getAuthorizationRealm();

        // user to retrieve default permission values
        final User userModel = m_coreContext.newUser();
        List<Group> groups = m_coreContext.getGroups();
        final Map<Integer, Group> groupsMap = new HashMap<Integer, Group>();
        for (Group group : groups) {
            groupsMap.put(group.getId(), group);
        }

        m_jdbcTemplate.query(QUERY, new RowCallbackHandler() {

            @Override
            public void processRow(ResultSet rs) throws SQLException {
                Element userEl = usersEl.addElement(ELEMENT_NAME_USER);
                String userId = rs.getString("user_id");
                String userName = StringUtils.defaultString(rs.getString("user_name"), StringUtils.EMPTY);
                String identity = AliasMapping.createUri(userName, domainName);
                String firstName = rs.getString("first_name");
                String lastName = rs.getString("last_name");
                Object[] names = {
                    firstName, lastName
                };
                String displayName = StringUtils.trimToNull(StringUtils.join(names, ' '));
                String contact = SipUri.format(displayName, userName, domainName);
                userEl.addElement(ELEMENT_NAME_IDENTITY).setText(identity);
                userEl.addElement(ELEMENT_NAME_USERNAME).setText(userName);
                final Element aliasesEl = userEl.addElement(ELEMENT_NAME_ALIASES);
                int aliasCount = rs.getInt("alias_count");
                if (aliasCount > 0) {
                    m_jdbcTemplate.query("select alias from user_alias where user_id=" + userId + ";",
                        new RowCallbackHandler() {
                            @Override
                            public void processRow(ResultSet rs) throws SQLException {
                                aliasesEl.addElement(ELEMENT_NAME_ALIAS).setText(rs.getString(ELEMENT_NAME_ALIAS));
                            }
                        });
                }
                int groupsCount = rs.getInt("groups");
                userModel.setGroups(new TreeSet<Group>());
                if (groupsCount > 0) {
                    // add groups to this user model
                    m_jdbcTemplate.query("SELECT u.group_id from user_group u inner join group_storage s "
                            + "on u.group_id = s.group_id WHERE user_id=" + userId
                            + " AND s.resource='user';", new RowCallbackHandler() {

                                @Override
                                public void processRow(ResultSet rs) throws SQLException {
                                    userModel.addGroup(groupsMap.get(rs.getInt("group_id")));
                                }
                            });
                }
                if (displayName != null) {
                    userEl.addElement(ELEMENT_NAME_DISPLAYNAME).setText(displayName);
                }
                userEl.addElement(ELEMENT_NAME_CONTACT).setText(contact);
                userEl.addElement(ELEMENT_NAME_PINTOKEN).setText(
                        defaultString(rs.getString(ELEMENT_NAME_PINTOKEN), EMPTY));
                String sipPassword = defaultString(rs.getString("sip_password"), EMPTY);
                String sipPasswordHash = Md5Encoder.digestPassword(userName, realm, sipPassword);
                userEl.addElement(ELEMENT_NAME_PASSTOKEN).setText(sipPasswordHash);

                Setting settings = userModel.getSettings();
                String inDirectory = getValueFromResultSet(rs, "in_directory",
                        getPermissionDefaultValue(userModel, Permission.CALL_PERMISSION_PATH,
                        PermissionName.AUTO_ATTENDANT_DIALING));
                userEl.addElement(ELEMENT_NAME_INDIRECTORY).setText(
                        Boolean.toString(inDirectory.equals(Permission.ENABLE)));
                String voicemail = getValueFromResultSet(rs, "has_voicemail",
                        getPermissionDefaultValue(userModel, Permission.CALL_PERMISSION_PATH,
                        PermissionName.VOICEMAIL));
                userEl.addElement(ELEMENT_NAME_HASVOICEMAIL).setText(
                        Boolean.toString(voicemail.equals(Permission.ENABLE)));
                String userBusyPrompt = getValueFromResultSet(rs, "user_busy_prompt",
                        settings.getSetting(MailboxPreferences.BUSY_PROMPT).getDefaultValue());
                if (userBusyPrompt != null) {
                    userEl.addElement(ELEMENT_NAME_USERBUSYPROMPT).setText(userBusyPrompt);
                }
                String voicemailTui = getValueFromResultSet(rs, "voicemail_tui",
                        settings.getSetting(MailboxPreferences.VOICEMAIL_TUI).getDefaultValue());
                if (voicemailTui != null) {
                    userEl.addElement(ELEMENT_NAME_VOICEMAILTUI).setText(voicemailTui);
                }
                String recordPrompts = getValueFromResultSet(rs, "record_prompts",
                        getPermissionDefaultValue(userModel, Permission.CALL_PERMISSION_PATH,
                        PermissionName.RECORD_SYSTEM_PROMPTS));
                userEl.addElement(ELEMENT_NAME_CANRECORDPROMPTS).setText(
                        Boolean.toString(recordPrompts.equals(Permission.ENABLE)));
                String tuiChangePin = getValueFromResultSet(rs, "tui_change_pin",
                        getPermissionDefaultValue(userModel, Permission.APPLICATION_PATH,
                        PermissionName.TUI_CHANGE_PIN));
                userEl.addElement(ELEMENT_NAME_CANTUICHANGEPIN).setText(
                        Boolean.toString(tuiChangePin.equals(Permission.ENABLE)));
                userEl.addElement(ELEMENT_NAME_MOH).setText(getMohValue(rs.getString("user_moh")));

                MailboxPreferences prefs = new MailboxPreferences();
                prefs.setEmailAddress(rs.getString("email_address"));
                prefs.setAlternateEmailAddress(rs.getString("alternate_email_address"));
                prefs.setActiveGreeting(ActiveGreeting.fromId(defaultIfEmpty(rs.getString("active_greeting"),
                        settings.getSetting(MailboxPreferences.ACTIVE_GREETING)
                        .getDefaultValue())));
                prefs.setBusyPrompt(userBusyPrompt);
                prefs.setVoicemailTui(VoicemailTuiType.fromValue(voicemailTui));
                prefs.setExternalMwi(defaultIfEmpty(rs.getString("external_mwi"), StringUtils.EMPTY));
                prefs.setAttachVoicemailToEmail(AttachType.fromValue(defaultIfEmpty(
                        rs.getString("primary_email_notif"),
                        settings.getSetting(MailboxPreferences.PRIMARY_EMAIL_NOTIFICATION)
                        .getDefaultValue())));
                prefs.setVoicemailToAlternateEmailNotification(AttachType.fromValue(defaultIfEmpty(
                        rs.getString("alt_email_notif"),
                        settings.getSetting(MailboxPreferences.ALT_EMAIL_NOTIFICATION)
                        .getDefaultValue())));
                prefs.setEmailFormat(MailFormat.valueOf(defaultIfEmpty(rs.getString("email_format"),
                        settings.getSetting(MailboxPreferences.PRIMARY_EMAIL_FORMAT)
                        .getDefaultValue())));
                prefs.setAlternateEmailFormat(MailFormat.valueOf(defaultIfEmpty(rs.getString("alt_email_format"),
                        settings.getSetting(MailboxPreferences.ALT_EMAIL_FORMAT)
                        .getDefaultValue())));
                prefs.setImapHost(defaultIfEmpty(rs.getString("imap_host"),
                        settings.getSetting(MailboxPreferences.IMAP_HOST).getDefaultValue()));
                prefs.setImapPort(defaultIfEmpty(rs.getString("imap_port"),
                        settings.getSetting(MailboxPreferences.IMAP_PORT).getDefaultValue()));
                String enabled = "1";
                prefs.setImapTLS(defaultIfEmpty(rs.getString("imap_tls"),
                        settings.getSetting(MailboxPreferences.IMAP_TLS).getDefaultValue()).equals(enabled));
                prefs.setImapAccount(defaultIfEmpty(rs.getString("imap_account"), StringUtils.EMPTY));
                prefs.setImapPassword(defaultIfEmpty(rs.getString("imap_password"), StringUtils.EMPTY));
                prefs.setIncludeAudioAttachment(defaultIfEmpty(rs.getString("primary_email_attach"),
                        settings.getSetting(MailboxPreferences.PRIMARY_EMAIL_ATTACH_AUDIO)
                        .getDefaultValue()).equals(enabled));
                prefs.setIncludeAudioAttachmentAlternateEmail(defaultIfEmpty(rs.getString("alt_email_attach"),
                        settings.getSetting(MailboxPreferences.ALT_EMAIL_ATTACH_AUDIO)
                        .getDefaultValue()).equals(enabled));
                generateEmailContact(userEl, prefs);
            }
        });

        // Load up the specified aliases
        List<AliasMapping> aliasMappings = (List<AliasMapping>) m_aliasProvider.getAliasMappings();
        // Generate the aliases
        for (AliasMapping am : aliasMappings) {
            generateAlias(usersEl, am);
        }
        return document;
    }

    private String getValueFromResultSet(ResultSet rs, String column, String defaultValue) throws SQLException {
        return defaultIfEmpty(rs.getString(column), defaultValue);
    }

    private String getPermissionDefaultValue(User user, String path, PermissionName permission) {
        return user.getSettings().getSetting(path + "/" + permission.getName()).getDefaultValue();
    }

    private void generateEmailContact(Element usersEl, MailboxPreferences mp) {
        Element primaryEmailEl = null;
        String emailAddress = mp.getEmailAddress();
        if (StringUtils.isNotBlank(emailAddress)) {
            primaryEmailEl = usersEl.addElement(ELEMENT_NAME_EMAIL);
            primaryEmailEl.addElement(ELEMENT_NAME_ADDRESS).setText(emailAddress);
            if (mp.isEmailNotificationEnabled()) {
                Element notificationEl = primaryEmailEl.addElement(ELEMENT_NAME_NOTIFICATION);
                notificationEl.setText(mp.getEmailFormat().name());
                notificationEl.addAttribute(ELEMENT_NAME_ATTACH_AUDIO,
                        Boolean.toString(mp.isIncludeAudioAttachment()));
            }
        }

        boolean imapServerConfigured = mp.isImapServerConfigured();
        if (imapServerConfigured) {
            if (primaryEmailEl == null) {
                primaryEmailEl = usersEl.addElement(ELEMENT_NAME_EMAIL);
            }
            Element imapEl = primaryEmailEl.addElement(ELEMENT_NAME_IMAP);
            imapEl.addAttribute(ELEMENT_NAME_SYNC, Boolean.toString(mp.isSynchronizeWithImapServer()));
            imapEl.addElement(ELEMENT_NAME_HOST).setText(mp.getImapHost());
            imapEl.addElement(ELEMENT_NAME_PORT).setText(mp.getImapPort());
            imapEl.addElement(ELEMENT_NAME_TLS).setText(Boolean.toString(mp.getImapTLS()));
            imapEl.addElement(ELEMENT_NAME_ACCOUNT).setText(StringUtils.defaultString(mp.getImapAccount()));
            // FIXME: this code is using default platform encoding - not safe
            String pwd = StringUtils.defaultString(mp.getImapPassword());
            String encodedPwd = new String(Base64.encodeBase64(pwd.getBytes()));
            imapEl.addElement(ELEMENT_NAME_PASSWD).setText(StringUtils.defaultString(encodedPwd));
        }

        String alternateEmailAddress = mp.getAlternateEmailAddress();
        if (StringUtils.isNotBlank(alternateEmailAddress)) {
            Element alternateEmailEl = usersEl.addElement(ELEMENT_NAME_EMAIL);
            alternateEmailEl.addElement(ELEMENT_NAME_ADDRESS).setText(alternateEmailAddress);
            if (mp.isEmailNotificationAlternateEnabled()) {
                Element alternateNotificationEl = alternateEmailEl.addElement(ELEMENT_NAME_NOTIFICATION);
                alternateNotificationEl.setText(mp.getAlternateEmailFormat().name());
                alternateNotificationEl.addAttribute(ELEMENT_NAME_ATTACH_AUDIO,
                        Boolean.toString(mp.isIncludeAudioAttachmentAlternateEmail()));
            }
        }
    }

    private void generateAlias(Element usersEl, AliasMapping am) {
        Element userEl = usersEl.addElement(ELEMENT_NAME_USER);
        String identity = am.getIdentity();
        String contact = am.getContact();
        userEl.addElement(ELEMENT_NAME_IDENTITY).setText(identity);
        userEl.addElement(ELEMENT_NAME_USERNAME).setText(identity.substring(0, identity.indexOf('@')));
        userEl.addElement(ELEMENT_NAME_CONTACT).setText(contact);
        userEl.addElement(ELEMENT_NAME_INDIRECTORY).setText("false");
    }

    public static String getMohValue(String mohValue) {
        String shortMoh = StringUtils.defaultString(mohValue, MusicOnHoldManagerImpl.SYSTEM_DEFAULT);
        if (shortMoh.equals(MusicOnHoldManagerImpl.FILES_SRC)) {
            return "f";
        }
        if (shortMoh.equals(MusicOnHoldManagerImpl.SOUNDCARD_SRC)) {
            return "c";
        }
        if (shortMoh.equals(MusicOnHoldManagerImpl.PERSONAL_FILES_SRC)) {
            return "p";
        }
        if (shortMoh.equals(MusicOnHoldManagerImpl.NONE)) {
            return "n";
        }
        return "d";
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setAliasProvider(AliasProvider aliasProvider) {
        m_aliasProvider = aliasProvider;
    }

    public void setConfigJdbcTemplate(JdbcTemplate template) {
        m_jdbcTemplate = template;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
