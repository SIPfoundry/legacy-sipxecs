/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm.attendant;

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.User;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public abstract class PersonalAttendantManager extends HibernateDaoSupport {

    public final PersonalAttendant loadPersonalAttendantForUser(User user) {
        PersonalAttendant pa = findPersonalAttendant(user);
        if (pa == null) {
            pa = new PersonalAttendant();
            pa.setUser(user);
            getHibernateTemplate().save(pa);
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
        getHibernateTemplate().saveOrUpdate(pa);
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
