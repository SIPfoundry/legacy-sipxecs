/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.commserver.imdb;

import java.util.UUID;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;

import com.mongodb.DBObject;

public class OpenAcd extends AbstractDataSetGenerator {
    @Override
    public boolean generate(Replicable entity, DBObject top) {
        if (entity.getName() != null) {
            top.put(MongoConstants.NAME, entity.getName());
        }

        top.put(OpenAcdContext.UUID,  UUID.randomUUID().toString());
        top.put(OpenAcdContext.TYPE, entity.getEntityName().toLowerCase());
        top.put(MongoConstants.REALM, getCoreContext().getAuthorizationRealm());

        return true;
    }

    @Override
    protected DataSet getType() {
        return DataSet.OPENACD;
    }
}
