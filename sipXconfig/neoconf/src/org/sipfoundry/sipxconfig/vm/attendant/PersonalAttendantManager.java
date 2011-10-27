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

    public final void removePersonalAttendantForUser(User user) {
        PersonalAttendant pa = findPersonalAttendant(user);
        if (pa != null) {
            getHibernateTemplate().delete(pa);
        }
    }

    public final void storePersonalAttendant(PersonalAttendant pa) {
        storePersonalAttendant(pa, true);
    }

    public final void storePersonalAttendant(PersonalAttendant pa, boolean writeFile) {
        getHibernateTemplate().saveOrUpdate(pa);
        if (writeFile) {
            writePersonalAttendant(pa);
        }
    }

    public final void clearPersonalAttendants() {
        List<PersonalAttendant> allPersonalAttendants = getHibernateTemplate().loadAll(PersonalAttendant.class);
        getHibernateTemplate().deleteAll(allPersonalAttendants);
    }

    public final void writeAllPersonalAttendants() {
        List<PersonalAttendant> all = getHibernateTemplate().loadAll(PersonalAttendant.class);
        for (PersonalAttendant pa : all) {
            writePersonalAttendant(pa);
        }
    }

    public abstract void writePersonalAttendant(PersonalAttendant pa);

    private PersonalAttendant findPersonalAttendant(User user) {
        Collection pas = getHibernateTemplate().findByNamedQueryAndNamedParam("personalAttendantForUser", "user",
                user);
        return (PersonalAttendant) DataAccessUtils.singleResult(pas);
    }


    public final void updatePersonalAttendantForUser(User user, String operatorValue) {
        PersonalAttendant pa = loadPersonalAttendantForUser(user);
        pa.setOperator(operatorValue);
        storePersonalAttendant(pa);
    }

}
