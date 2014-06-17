/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 */

package org.sipfoundry.sipxconfig.systemaudit;

import java.lang.reflect.InvocationTargetException;
import java.util.Map;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.beanutils.PropertyUtilsBean;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserIpAddress;
import org.sipfoundry.sipxconfig.security.SipxAuthenticationDetails;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.context.SecurityContextHolder;

/**
 * Parent class for all the ConfigChangeHandler classes.
 */
public abstract class AbstractSystemAuditHandler {

    public static final String LOCALHOST = "localhost";

    private ConfigChangeContext m_configChangeContext;
    private CoreContext m_coreContext;

    @Required
    public void setConfigChangeContext(ConfigChangeContext configChangeContext) {
        m_configChangeContext = configChangeContext;
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public ConfigChange buildConfigChange(ConfigChangeAction configChangeAction,
            ConfigChangeType configChangeType) throws SystemAuditException {
        // default userName is "superadmin" to cover the cases where the update is done by the system
        String userName = User.SUPERADMIN;
        String ipAddress = LOCALHOST;
        Authentication authentication = SecurityContextHolder.getContext().getAuthentication();
        if (authentication != null) {
            SipxAuthenticationDetails authDetails = (SipxAuthenticationDetails) authentication.getDetails();
            if (authDetails != null && authDetails.getRemoteAddress() != null) {
                ipAddress = authDetails.getRemoteAddress();
            }
            Object userDetails = authentication.getPrincipal();
            if (userDetails instanceof UserDetailsImpl) {
                userName = ((UserDetailsImpl) userDetails).getUsername();
            }
        }
        ConfigChange configChange = buildConfigChange(configChangeAction, configChangeType, userName,
                ipAddress);
        return configChange;
    }

    public ConfigChange buildConfigChange(ConfigChangeAction configChangeAction,
            ConfigChangeType configChangeType, String userName, String ipAddress) {
        ConfigChange configChange = new ConfigChange();
        configChange.setConfigChangeAction(configChangeAction);
        configChange.setConfigChangeType(configChangeType);
        UserIpAddress userIpAddress = m_coreContext.getUserIpAddress(userName, ipAddress);
        if (userIpAddress == null) {
            userIpAddress = new UserIpAddress(userName, ipAddress);
        }
        configChange.setUserIpAddress(userIpAddress);

        return configChange;
    }

    /**
     * Utility method to protect against NullPointers
     */
    protected Object getValue(Object[] objList, int counter) {
        Object value = null;
        if (objList != null) {
            value = objList[counter];
        }
        return value;
    }

    /**
     * Utility method to return of the provided Object. If the object is an
     * instance of NamedObject it will return getName(), if not it will return
     * toString().
     */
    protected String getObjectName(Object object) {
        if (object instanceof NamedObject) {
            NamedObject namedObject = (NamedObject) object;
            return namedObject.getName();
        }
        return object.toString();
    }

    /**
     * Utility method, checks if the child object is contained in the parent
     * object
     */
    protected boolean isChildContainedInParent(Object child, Object parent) throws IllegalAccessException,
            InvocationTargetException, NoSuchMethodException, SystemAuditException {
        Map map = BeanUtils.describe(parent);
        PropertyUtilsBean propUtils = new PropertyUtilsBean();
        for (Object propNameObject : map.keySet()) {
            Object propValue = propUtils.getNestedProperty(parent, propNameObject.toString());
            if (propValue != null && child.equals(propValue)) {
                return true;
            }
        }
        return false;
    }

    public ConfigChangeContext getConfigChangeContext() {
        return m_configChangeContext;
    }

    public CoreContext getCoreContext() {
        return m_coreContext;
    }

}
