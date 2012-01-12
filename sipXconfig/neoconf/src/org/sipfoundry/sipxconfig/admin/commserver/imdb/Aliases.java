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

import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.openacd.OpenAcdExtension;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;

import static org.sipfoundry.commons.mongo.MongoConstants.ALIASES;

public abstract class Aliases extends AbstractDataSetGenerator {
    public static final String FAX_EXTENSION_PREFIX = "~~ff~";

    public abstract SipxFreeswitchService getSipxFreeswitchService();

    @Override
    protected DataSet getType() {
        return DataSet.ALIAS;
    }

    @Override
    public boolean generate(Replicable entity, DBObject top) {
        if (entity instanceof User || entity instanceof OpenAcdExtension) {
            top.put(ALIASES, entity.getAliasMappings(getCoreContext().getDomainName(), getSipxFreeswitchService()));
        } else {
            top.put(ALIASES, entity.getAliasMappings(getCoreContext().getDomainName()));
        }
        return true;
    }

}
