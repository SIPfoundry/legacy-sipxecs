/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
 */
package org.sipfoundry.sipxconfig.vm.attendant;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.User;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public abstract class PersonalAttendantManager extends HibernateDaoSupport {

    public PersonalAttendant loadPersonalAttendantForUser(User user) {
        PersonalAttendant pa = findPersonalAttendant(user);
        if (pa == null) {
            pa = new PersonalAttendant();
            pa.setUser(user);
            getHibernateTemplate().merge(pa);
        }
        return pa;
    }

    public PersonalAttendant getPersonalAttendantForUser(User user) {
        return findPersonalAttendant(user);
    }

    public final void removePersonalAttendantForUser(User user) {
        PersonalAttendant pa = findPersonalAttendant(user);
        if (pa != null) {
            getHibernateTemplate().delete(pa);
        }
    }

    public final void storePersonalAttendant(PersonalAttendant pa) {
        if (pa.isNew()) {
            getHibernateTemplate().save(pa);
        } else {
            getHibernateTemplate().merge(pa);
        }
    }

    public final void clearPersonalAttendants() {
        List<PersonalAttendant> allPersonalAttendants = getHibernateTemplate().loadAll(PersonalAttendant.class);
        getHibernateTemplate().deleteAll(allPersonalAttendants);
    }

    private PersonalAttendant findPersonalAttendant(User user) {
        Collection pas = getHibernateTemplate().findByNamedQueryAndNamedParam("personalAttendantForUser", "user",
                user);
        return (PersonalAttendant) DataAccessUtils.singleResult(pas);
    }

}
