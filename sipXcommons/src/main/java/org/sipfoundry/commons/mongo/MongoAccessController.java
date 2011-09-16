/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.mongo;

import java.net.UnknownHostException;

import com.mongodb.CommandResult;
import com.mongodb.DB;
import com.mongodb.Mongo;

/**
 * to be used for access to mongo db
 * 
 */
public enum MongoAccessController {
    INSTANCE;

    private int PORT = 27017;
    private Mongo m_mongoInstance;

    public DB getDatabase(String dbName) throws UnknownHostException {
        return getMongo().getDB(dbName);
    }

    public void dropDatabase(String dbName) throws UnknownHostException {
        getMongo().dropDatabase(dbName);
    }

    private Mongo getMongo() throws UnknownHostException {
        if (m_mongoInstance == null) {
            m_mongoInstance = new Mongo();
        }
        return m_mongoInstance;
    }

    public ResyncResult resync(String ipAddress) throws UnknownHostException {
        Mongo mongo = null;
        try {
            mongo = new Mongo(ipAddress, PORT);
            DB adminDb = mongo.getDB("admin");
            CommandResult cr = adminDb.command("resync");
            return new ResyncResult((Double) cr.get("ok"), cr.get("errmsg"));
        } finally {
            if (mongo != null) {
                mongo.close();
            }
        }
    }
}
