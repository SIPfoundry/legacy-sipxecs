package org.sipfoundry.openfire.plugin.listener;

import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.regex.Pattern;

import org.sipfoundry.openfire.plugin.job.JobFactory;
import org.sipfoundry.openfire.sync.MongoOperation;
import org.sipfoundry.openfire.sync.listener.MongoOplogListener;

import com.mongodb.DBObject;
import com.mongodb.QueryBuilder;

public class ImdbOplogListener extends MongoOplogListener<JobFactory> {
    private static final String WATCHED_NAMESPACE = "imdb.entity";
    private static final String ID = "_id";
    private static final Pattern USER_PATTERN = Pattern.compile("^User\\d+$");
    private static final Pattern GROUP_PATTERN = Pattern.compile("^Group\\d+$");
    private static final List<Pattern> WATCHED_ENTITIES = Arrays.asList(USER_PATTERN, GROUP_PATTERN);
    private static final List<MongoOperation> WATCHED_OPERATIONS = Arrays.asList(MongoOperation.INSERT,
            MongoOperation.UPDATE, MongoOperation.DELETE);

    public ImdbOplogListener(JobFactory factory) {
        setJobFactory(factory);
    }

    @Override
    protected DBObject buildOpLogQuery() {
        DBObject nsQuery = QueryBuilder.start(NAMESPACE).is(WATCHED_NAMESPACE).get();
        DBObject ent1Query = QueryBuilder.start(RECORD + "." + ID).in(WATCHED_ENTITIES).get();
        DBObject ent2Query = QueryBuilder.start(RECORD2 + "." + ID).in(WATCHED_ENTITIES).get();
        DBObject entQuery = QueryBuilder.start().or(ent1Query, ent2Query).get();

        return QueryBuilder.start().and(nsQuery, entQuery).get();
    }

    @Override
    protected Collection<MongoOperation> getWatchedOperations() {
        return WATCHED_OPERATIONS;
    }
}
