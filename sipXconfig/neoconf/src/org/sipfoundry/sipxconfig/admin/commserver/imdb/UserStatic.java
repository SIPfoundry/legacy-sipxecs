/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class UserStatic extends DataSetGenerator {
    public static final String EXTERNAL_MWI = "voicemail/mailbox/external-mwi";
    public static final String STATIC = "stc";

    @Override
    protected DataSet getType() {
        return DataSet.USER_STATIC;
    }

    public void generate() {
        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                generate(user);
            }
        };
        forAllUsersDo(getCoreContext(), closure);
    }

    public void generate(Replicable entity) {
        if (entity instanceof User) {
            DBObject top = findOrCreate(entity);
            User user = (User) entity;
            String domainName = getSipDomain();
            String externalMwi = user.getSettingValue(EXTERNAL_MWI);
            if (externalMwi != null) {
                top.put(STATIC, new UserStaticMapping(domainName, user.getUserName(), externalMwi));
            } else {
                top.removeField(STATIC);
            }
            getDbCollection().save(top);
        }
    }

}
