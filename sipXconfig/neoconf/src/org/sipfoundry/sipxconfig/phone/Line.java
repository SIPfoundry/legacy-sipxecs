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

import java.util.Set;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.BeanWithGroupsModel;
import org.sipfoundry.sipxconfig.setting.Setting;

import static org.sipfoundry.sipxconfig.common.SipUri.DEFAULT_SIP_PORT;
import static org.sipfoundry.sipxconfig.common.SipUri.formatIgnoreDefaultPort;
import static org.sipfoundry.sipxconfig.common.SipUri.parsePort;

public class Line extends BeanWithGroups {

    private Phone m_phone;

    private User m_user;

    private boolean m_initialized;

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
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
}
