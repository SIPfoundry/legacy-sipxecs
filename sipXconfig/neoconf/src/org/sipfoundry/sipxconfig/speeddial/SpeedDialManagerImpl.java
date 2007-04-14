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
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;

public class SpeedDialManagerImpl extends SipxHibernateDaoSupport<SpeedDial> implements
        SpeedDialManager {

    private CoreContext m_coreContext;

    private SipxReplicationContext m_replicationContext;

    private ResourceLists m_resourceLists;

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
        List<SpeedDial> speeddials = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "speedDialForUserId", "userId", userId);
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
        protected void onUserDelete(User user) {
            List<SpeedDial> speedDials = findSpeedDialForUserId(user.getId());
            if (!speedDials.isEmpty()) {
                getHibernateTemplate().deleteAll(speedDials);
                activateResourceList();
            }
        }
    }

    /**
     * Generates a new Resource List XML file for RLS (Resource List Server)
     */
    public void activateResourceList() {
        m_resourceLists.generate(this);
        m_replicationContext.replicate(m_resourceLists);
    }

    public List<DialingRule> getDialingRules() {
        DialingRule[] rules = new DialingRule[] {
            new RlsRule()
        };
        return Arrays.asList(rules);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setResourceLists(ResourceLists resourceLists) {
        m_resourceLists = resourceLists;
    }

    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }
}
