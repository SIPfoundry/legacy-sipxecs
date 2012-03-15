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
package org.sipfoundry.authcode;

import static org.sipfoundry.commons.mongo.MongoConstants.ID;

import java.util.regex.Pattern;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;

import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.QueryBuilder;

public final class AuthCodeManager {

    private AuthCodeManager() {
        // utility, change it when porting to Spring
    }

    public static AuthCodeConfig getAuthCode(String code) {
        DBCollection entityCol = UnfortunateLackOfSpringSupportFactory.getImdb().getCollection("entity");
        Pattern codePattern = Pattern.compile("AuthCode.*");
        DBObject query = QueryBuilder.start(ID).is(codePattern).and(MongoConstants.AUTH_CODE).is(code).get();
        DBObject result = entityCol.findOne(query);
        if (result != null) {
            AuthCodeConfig conf = new AuthCodeConfig();
            conf.setAuthCode(getStringValue(result, MongoConstants.AUTH_CODE));
            conf.setAuthName(getStringValue(result, MongoConstants.UID));
            conf.setAuthPassword(getStringValue(result, MongoConstants.PASSTOKEN));
            return conf;
        }
        return null;
    }

    private static String getStringValue(DBObject obj, String key) {
        if (obj.keySet().contains(key)) {
            if (obj.get(key) != null) {
                return obj.get(key).toString();
            }
        }
        return null;
    }

}
