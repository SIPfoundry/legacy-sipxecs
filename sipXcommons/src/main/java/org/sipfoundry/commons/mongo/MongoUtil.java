/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.commons.mongo;

import static java.lang.String.format;

import org.apache.commons.lang.StringUtils;
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
    
    public static void checkForError(BasicBSONObject o) {
        int ok = o.getInt("ok");
        if (ok == 0) {
            String what = o.getString("assertion");
            if (StringUtils.isBlank(what)) {
                what = o.getString("errmsg");
                if (StringUtils.isBlank(what)) {
                    what = "undetermined error";
                }                
            }
            throw new MongoCommandException(what);
        }
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
