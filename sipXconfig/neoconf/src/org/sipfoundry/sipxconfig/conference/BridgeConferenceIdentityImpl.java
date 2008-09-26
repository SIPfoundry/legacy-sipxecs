/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.io.Serializable;

import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class BridgeConferenceIdentityImpl extends HibernateDaoSupport  implements BridgeConferenceIdentity {
    private Bridge m_bridge;

    public Conference load(Class c, Serializable id) {
        Conference conf = (Conference) getHibernateTemplate().load(c, id);
        if (conf.getBridge().getId().equals(m_bridge.getId())) {
            return  conf;
        } else {
            return null;
        }
    }

    public void setBridge(Bridge bridge) {
        this.m_bridge = bridge;
    }

}
