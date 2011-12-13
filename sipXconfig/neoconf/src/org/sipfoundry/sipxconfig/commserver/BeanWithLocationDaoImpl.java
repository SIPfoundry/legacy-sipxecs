/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver;

import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;

public class BeanWithLocationDaoImpl<T extends BeanWithLocation> extends SipxHibernateDaoSupport<T> implements
        BeanWithLocationDao<T> {

    @Override
    public List<T> findAll() {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public T findOne(Location location) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public void upsert(T bean) {
        // TODO Auto-generated method stub

    }

    @Override
    public List<T> findAll(Location location) {
        // TODO Auto-generated method stub
        return null;
    }

}
