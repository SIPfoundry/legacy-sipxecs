/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 */
package org.sipfoundry.openfire.sync.listener;

import static org.sipfoundry.commons.mongo.MongoConstants.ID;

import java.net.UnknownHostException;
import java.util.Collection;

import org.apache.log4j.Logger;
import org.bson.types.BSONTimestamp;
import org.sipfoundry.commons.mongo.MongoFactory;
import org.sipfoundry.openfire.sync.MongoOperation;
import org.sipfoundry.openfire.sync.job.AbstractJobFactory;
import org.sipfoundry.openfire.sync.job.Job;
import org.sipfoundry.openfire.sync.job.QueueManager;

import com.mongodb.Bytes;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.Mongo;

public abstract class MongoOplogListener<T extends AbstractJobFactory> implements Runnable {
    private static Logger logger = Logger.getLogger(MongoOplogListener.class);

    protected static final String RECORD = "o";
    protected static final String RECORD2 = "o2";
    protected static final String NAMESPACE = "ns";
    private static final String OPERATION = "op";
    private static final String TIMESTAMP = "ts";
    private static final String SET_OP = "$set";

    private T m_jobFactory;

    public void setJobFactory(T factory) {
        m_jobFactory = factory;
    }

    @Override
    public void run() {
        logger.debug("Running " + this.getClass());
        Mongo m = null;
        try {
            m = MongoFactory.fromConnectionFile();
        } catch (UnknownHostException ex) {
            logger.error("Error running mongo opLog listener", ex);
            return;
        }
        DB db = m.getDB("local");
        DBCollection col = db.getCollection("oplog.rs");
        long seconds = System.currentTimeMillis() / 1000;
        DBObject query = buildOpLogQuery();
        System.out.println("Query: " + query);
        DBCursor cur = col.find(query).addOption(Bytes.QUERYOPTION_TAILABLE).addOption(Bytes.QUERYOPTION_AWAITDATA);

        while (cur.hasNext()) {
            DBObject object = cur.next();
            try {
                int time = ((BSONTimestamp) object.get(TIMESTAMP)).getTime();

                if (time > seconds) {
                    logger.debug(String.format("Got operation: %s", object));
                    MongoOperation op = MongoOperation.fromString((String) object.get(OPERATION));

                    if (getWatchedOperations().contains(op)) {
                        DBObject record = (DBObject) object.get(RECORD);
                        Object id = record.get(ID);

                        if (id == null) {
                            // this is an update, the id is in a different place
                            DBObject otherRecord = (DBObject) object.get(RECORD2);
                            id = otherRecord.get(ID);
                            // if it's a partial update, consider only the update part
                            DBObject partialUpdate = (DBObject) record.get(SET_OP);
                            if (partialUpdate != null) {
                                record = partialUpdate;
                            }
                        }
                        Job j = m_jobFactory.createJob(op, record, id);

                        // job can be null if the affected object is not in use (e.g. user
                        // update for an offline user - updated data will be read when the
                        // user comes online)
                        if (j != null) {
                            QueueManager.submitJob(j);
                        }
                    }
                }
            } catch (Exception ex) {
                // this is a guard: no matter what fails, this worker loop must continue
                // handling changes
                logger.error("Error processing change: " + object, ex);
            }
        }

        cur.close();
    }

    protected abstract DBObject buildOpLogQuery();

    protected abstract Collection<MongoOperation> getWatchedOperations();
}
