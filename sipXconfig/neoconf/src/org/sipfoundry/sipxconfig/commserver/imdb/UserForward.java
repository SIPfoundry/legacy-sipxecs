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

import org.sipfoundry.sipxconfig.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.commons.mongo.MongoConstants.CFWDTIME;

public class UserForward extends AbstractDataSetGenerator {

    @Override
    protected DataSet getType() {
        return DataSet.USER_FORWARD;
    }

    @Override
    public boolean generate(Replicable entity, DBObject top) {
        if (entity instanceof User) {
            generateUser((User) entity, top);
            return true;
        }
        return false;
    }

    private void generateUser(User user, DBObject top) {
        top.put(CFWDTIME, user.getSettingTypedValue(CallSequence.CALL_FWD_TIMER_SETTING));
    }

}
