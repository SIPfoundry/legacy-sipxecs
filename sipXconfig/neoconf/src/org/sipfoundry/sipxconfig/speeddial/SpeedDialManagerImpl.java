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

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxRlsService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

public class SpeedDialManagerImpl extends SipxHibernateDaoSupport<SpeedDial> implements SpeedDialManager {

    private CoreContext m_coreContext;

    private SipxServiceManager m_sipxServiceManager;

    private ServiceConfigurator m_serviceConfigurator;

    public SpeedDial getSpeedDialForUserId(Integer userId, boolean create) {
        List<SpeedDial> speeddials = findSpeedDialForUserId(userId);
        if (!speeddials.isEmpty()) {
            return speeddials.get(0);
        }
        if (!create) {
            return null;
        }
        SpeedDial speedDial = new SpeedDial();
        speedDial.setUser(m_coreContext.loadUser(userId));
        saveSpeedDial(speedDial);
        return speedDial;
    }

    private List<SpeedDial> findSpeedDialForUserId(Integer userId) {
        List<SpeedDial> speeddials = getHibernateTemplate().findByNamedQueryAndNamedParam("speedDialForUserId",
                "userId", userId);
        return speeddials;
    }

    public void saveSpeedDial(SpeedDial speedDial) {
        getHibernateTemplate().saveOrUpdate(speedDial);
        activateResourceList();
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        @Override
        protected void onUserDelete(User user) {
            List<SpeedDial> speedDials = findSpeedDialForUserId(user.getId());
            if (!speedDials.isEmpty()) {
                getHibernateTemplate().deleteAll(speedDials);
                activateResourceList();
            }
        }
    }

    public List<DialingRule> getDialingRules() {
        DialingRule[] rules = new DialingRule[] {
            new RlsRule()
        };
        return Arrays.asList(rules);
    }

    public void activateResourceList() {
        SipxService rlsService = m_sipxServiceManager.getServiceByBeanId(SipxRlsService.BEAN_ID);
        m_serviceConfigurator.replicateServiceConfig(rlsService);
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public void clear() {
        Collection c = getHibernateTemplate().loadAll(SpeedDial.class);
        getHibernateTemplate().deleteAll(c);
    }
}
