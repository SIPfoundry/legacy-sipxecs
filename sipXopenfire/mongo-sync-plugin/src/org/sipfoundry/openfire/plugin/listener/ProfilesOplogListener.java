package org.sipfoundry.openfire.plugin.listener;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.log4j.Logger;
import org.sipfoundry.openfire.plugin.MongoOperation;

import com.mongodb.DBObject;
import com.mongodb.QueryBuilder;

public class ProfilesOplogListener extends MongoOplogListener {
    private static final Logger log = Logger.getLogger(ProfilesOplogListener.class);
    private static final String WATCHED_NAMESPACE = "profiles.fs.files";
    private static final List<MongoOperation> WATCHED_OPERATIONS = Arrays.asList(MongoOperation.INSERT,
            MongoOperation.DELETE);

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
