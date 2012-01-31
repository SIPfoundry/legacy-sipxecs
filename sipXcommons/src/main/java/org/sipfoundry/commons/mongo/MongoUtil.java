/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.commons.mongo;

import static java.lang.String.format;

import org.bson.BasicBSONObject;

import com.mongodb.CommandResult;
import com.mongodb.DB;

/**
 * Utility function when dealing with mongo
 */
public final class MongoUtil {
    
    private MongoUtil() {        
    }

    @SuppressWarnings("serial")
    public static class MongoCommandException extends RuntimeException {
        public MongoCommandException(String msg) {
            super(msg);
        }
    }

    /**
     * Running java script commands. With throw MongoCommandException if
     * command wasn't successful.
     * 
     * Example:
     *  BasicBSONObject ret = MongoUtil.runCommand(m_db, "rs.config()");
     */
    public static BasicBSONObject runCommand(DB db, String command) {
        CommandResult status = db.doEval(command);
        if (!status.ok()) {
            String msg = format("Cannot run command '%s'. Result '%s'.", command, status);
            throw new MongoCommandException(msg);
        }
        return getObject(status, "retval");
    }

    /**
     * Get an object from a nest result set of objects
     * 
     * Example:
     *  BasicBSONObject city = getObject(m_db, "country", "state", "city");
     */
    public static BasicBSONObject getObject(BasicBSONObject o, String... keys) {
        BasicBSONObject s = o;
        for (String key : keys) {
            s = (BasicBSONObject) s.get(key);
            if (s == null) {
                // make this a safe call, and just return null if full
                // tree isn't there
                return s;
            }
        }
        return s;
    }
}
