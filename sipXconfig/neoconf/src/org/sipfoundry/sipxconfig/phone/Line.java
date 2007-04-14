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

import org.sipfoundry.sipxconfig.common.DataCollectionItem;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.BeanWithGroupsModel;
import org.sipfoundry.sipxconfig.setting.Setting;

public class Line extends BeanWithGroups implements DataCollectionItem {

    private Phone m_phone;
    
    private User m_user;
    
    private int m_position;
    
    private boolean m_initialized;
    
    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public String getDisplayLabel() {
        User u = getUser();
        if (u != null) {
            return u.getUserName();
        } 
        return getLineInfo().getUserId();
    }

    public int getPosition() {
        return m_position;
    }

    public void setPosition(int position) {
        m_position = position;
    }
    
    protected Setting loadSettings() {
        Phone phone = getPhone();
        Setting settings = phone.loadLineSettings();
        
        // kludge - not obvious place to initialize, but latest place
        initialize();
        
        return settings;
    }
    
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
        String uri = null;
        User u = getUser();
        if (u != null) {
            uri = u.getUri(getPhoneContext().getPhoneDefaults().getDomainName());
        } else {
            LineInfo info = getPhone().getLineInfo(this);
            int port = SipUri.parsePort(info.getRegistrationServerPort(), SipUri.DEFAULT_SIP_PORT);
            uri = SipUri.formatIgnoreDefaultPort(info.getDisplayName(), info.getUserId(), 
                    info.getRegistrationServer(), port);
        }
        
        return uri;
    }
    
    /**
     * Extract basic values from a phone line that most phones should understand.
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
