/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.login;

import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.dao.support.DataAccessUtils;

public class PrivateUserKeyManagerImpl extends SipxHibernateDaoSupport<PrivateUserKey> implements
        PrivateUserKeyManager {
    @Override
    public User getUserFromPrivateKey(String privateKey) {
        List users = getHibernateTemplate().findByNamedQueryAndNamedParam("userForPrivateKey", "key", privateKey);
        return (User) DataAccessUtils.singleResult(users);
    }

    @Override
    public String getPrivateKeyForUser(User user) {
        String key = getKeyForUser(user);
        if (key != null) {
            return key;
        }
        return createUserPrivateKey(user);
    }

    private String createUserPrivateKey(User user) {
        PrivateUserKey privateUserKey = new PrivateUserKey(user);
        getHibernateTemplate().saveOrUpdate(privateUserKey);
        return privateUserKey.getKey();
    }

    private String getKeyForUser(User user) {
        List keys = getHibernateTemplate().findByNamedQueryAndNamedParam("privateKeyForUser", "user", user);
        return (String) DataAccessUtils.singleResult(keys);
    }
}
