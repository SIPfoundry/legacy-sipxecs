/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import static org.sipfoundry.sipxconfig.common.SipUri.DEFAULT_SIP_PORT;
import static org.sipfoundry.sipxconfig.common.SipUri.formatIgnoreDefaultPort;
import static org.sipfoundry.sipxconfig.common.SipUri.parsePort;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.BeanWithGroupsModel;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public class Line extends BeanWithGroups implements NamedObject, SystemAuditable {
    private static final String COMMA = ",";
    private static final String EQUALS = "=";

    private Phone m_phone;

    private User m_user;

    private boolean m_initialized;

    private List<String> m_paths = new ArrayList<String>();

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public void setPaths(List<String> paths) {
        m_paths = paths;
    }

    public String getUserName() {
        if (m_user == null) {
            // for external lines user name is stored in DB already
            return null;
        }
        return m_user.getUserName();
    }

    /**
     * Get the string to be used as the "authentication user"
     *
     * For internal lines it looks like this: [user]/[phone-serial-no] For external lines it's the
     * same as username.
     */
    public String getAuthenticationUserName() {
        if (m_user == null) {
            // no user -- external line - use default value stored in DB
            return null;
        }
        return String.format("%s/%s", getUserName(), getPhone().getSerialNumber());
    }

    public String getDisplayLabel() {
        User u = getUser();
        if (u != null) {
            return u.getUserName();
        }
        return getLineInfo().getUserId();
    }

    @Override
    protected Setting loadSettings() {
        Phone phone = getPhone();
        Setting settings = phone.loadLineSettings();

        // HACK: not obvious place to initialize, but latest place
        initialize();
        return settings;
    }

    @Override
    public Set getGroups() {
        // Use phone groups until we can justify lines
        // having their own groups and work out a reasonable UI
        Set groups = getPhone().getGroups();

        return groups;
    }

    public PhoneContext getPhoneContext() {
        return getPhone().getPhoneContext();
    }

    public Phone getPhone() {
        return m_phone;
    }

    public void setPhone(Phone phone) {
        m_phone = phone;
    }

    public String getUri() {
        User u = getUser();
        if (u != null) {
            String domainName = getPhoneContext().getPhoneDefaults().getDomainName();
            return u.getUri(domainName);
        }
        LineInfo info = getPhone().getLineInfo(this);
        int port = parsePort(info.getRegistrationServerPort(), DEFAULT_SIP_PORT);
        return formatIgnoreDefaultPort(info.getDisplayName(), info.getUserId(), info.getRegistrationServer(), port);
    }

    public String getAddrSpec() {
        User u = getUser();
        if (u != null) {
            String domainName = getPhoneContext().getPhoneDefaults().getDomainName();
            return u.getAddrSpec(domainName);
        }
        LineInfo info = getPhone().getLineInfo(this);
        int port = parsePort(info.getRegistrationServerPort(), DEFAULT_SIP_PORT);
        return formatIgnoreDefaultPort(info.getUserId(), info.getRegistrationServer(), port);
    }

    /**
     * Extract basic values from a phone line that most phones should understand.
     *
     * @return as much information as phone has or understands
     */
    public LineInfo getLineInfo() {
        return getPhone().getLineInfo(this);
    }

    /**
     * Set basic values on a phone line that most phones should understand
     */
    public void setLineInfo(LineInfo lineInfo) {
        getPhone().setLineInfo(this, lineInfo);
    }

    public String getAdditionalLineSettings() {
        List<String> settings = new ArrayList<String>();
        addSetting(settings, m_paths);

        return StringUtils.join(settings, COMMA);
    }

    public void setAdditionalLineSettings(String additionalSettings) {
        List<String> settings = Arrays.asList(StringUtils.split(additionalSettings, COMMA));
        settings = Arrays.asList(StringUtils.split(additionalSettings, COMMA));
        for (String setting : settings) {
            setSettingValue(StringUtils.substringBefore(setting, EQUALS),
                    StringUtils.substringAfter(setting, EQUALS));
        }
    }

    private void addSetting(List<String> settingsList, List<String> paths) {
        Setting settings = getSettings();
        if (paths != null && settings != null) {
            for (String path : paths) {
                Setting setting = settings.getSetting(path);
                String settingValue = (null == setting ? null : (setting.getValue() == null ? null : setting
                        .getValue()));
                if (!StringUtils.isEmpty(settingValue)) {
                    settingsList.add(path + EQUALS + settingValue);
                }
            }
        }
    }

    @Override
    public synchronized void initialize() {
        if (!m_initialized && m_phone != null) {
            m_phone.initializeLine(this);
            m_initialized = true;

            BeanWithGroupsModel model = (BeanWithGroupsModel) getSettingModel2();
            // passed collection is not copied
            model.setGroups(m_phone.getGroups());
        }
    }

    @Override
    public String getEntityIdentifier() {
        return getUri();
    }

    @Override
    public ConfigChangeType getConfigChangeType() {
        return ConfigChangeType.LINE;
    }

    public boolean isNotSpecialPhoneProvisionUserLine() {
        boolean notProv = true;
        if (getUser() != null && SpecialUserType.PHONE_PROVISION.getUserName().equals(getUser().getUserName())) {
            notProv = false;
        }
        return notProv;
    }

    @Override
    public String getName() {
        return getUri();
    }

    @Override
    public void setName(String name) {
        // Do Nothing
    }

}
