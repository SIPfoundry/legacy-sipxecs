/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.openfire.plugin.listener;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.log4j.Logger;
import org.sipfoundry.openfire.plugin.job.JobFactory;
import org.sipfoundry.openfire.sync.MongoOperation;
import org.sipfoundry.openfire.sync.listener.MongoOplogListener;

import com.mongodb.DBObject;
import com.mongodb.QueryBuilder;

public class ProfilesOplogListener extends MongoOplogListener<JobFactory> {
    private static final Logger log = Logger.getLogger(ProfilesOplogListener.class);
    private static final String WATCHED_NAMESPACE = "profiles.fs.files";
    private static final List<MongoOperation> WATCHED_OPERATIONS = Arrays.asList(MongoOperation.INSERT,
            MongoOperation.DELETE);

    public ProfilesOplogListener(JobFactory factory) {
        setJobFactory(factory);
    }

    @Override
    protected DBObject buildOpLogQuery() {
        DBObject nsQuery = QueryBuilder.start(NAMESPACE).is(WATCHED_NAMESPACE).get();
        log.debug("Oplog query: " + nsQuery.toString());
        return nsQuery;
    }

    @Override
    protected Collection<MongoOperation> getWatchedOperations() {
        return WATCHED_OPERATIONS;
    }

}
