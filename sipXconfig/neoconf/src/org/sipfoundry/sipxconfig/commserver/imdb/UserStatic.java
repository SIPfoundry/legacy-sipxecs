/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.commons.mongo.MongoConstants.STATIC;

public class UserStatic extends AbstractDataSetGenerator {
    public static final String EXTERNAL_MWI = "voicemail/mailbox/external-mwi";

    @Override
    protected DataSet getType() {
        return DataSet.USER_STATIC;
    }

    @Override
    public void generate(Replicable entity, DBObject top) {
        if (entity instanceof User) {
            User user = (User) entity;
            String domainName = getSipDomain();
            String externalMwi = user.getSettingValue(EXTERNAL_MWI);
            if (externalMwi != null) {
                top.put(STATIC, new UserStaticMapping(domainName, user.getUserName(), externalMwi));
            } else {
                top.removeField(STATIC);
            }
        }
    }

}
