/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * 
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import java.util.List;

import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class SbcManagerImpl extends HibernateDaoSupport implements SbcManager {

    public Sbc loadDefaultSbc() {
        List sbcs = getHibernateTemplate().loadAll(Sbc.class);
        Sbc sbc = (Sbc) DataAccessUtils.singleResult(sbcs);
        if (sbc == null) {
            sbc = new Sbc();
            sbc.setRoutes(new SbcRoutes());
            saveDefaultSbc(sbc);
        }
        return sbc;
    }

    public void saveDefaultSbc(Sbc sbc) {
        getHibernateTemplate().saveOrUpdate(sbc);
    }

}
