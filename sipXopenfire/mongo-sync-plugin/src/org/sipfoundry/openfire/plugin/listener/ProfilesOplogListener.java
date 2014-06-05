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
