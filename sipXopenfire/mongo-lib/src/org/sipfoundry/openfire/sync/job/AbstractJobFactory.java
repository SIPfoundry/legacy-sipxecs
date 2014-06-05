package org.sipfoundry.openfire.sync.job;

import org.sipfoundry.openfire.sync.MongoOperation;

import com.mongodb.DBObject;

public abstract class AbstractJobFactory {
    public abstract Job createJob(MongoOperation op, DBObject dbObj, Object id);
}
