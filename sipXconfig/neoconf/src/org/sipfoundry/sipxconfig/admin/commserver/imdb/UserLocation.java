/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class UserLocation extends DataSetGenerator {
    public static final String LOCATION = "loc";

    @Override
    protected DataSet getType() {
        return DataSet.USER_LOCATION;
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
            Branch site = user.getSite();
            if (site != null) {
                top.put(LOCATION, site.getName());
            }
            getDbCollection().save(top);
        }
    }

}
