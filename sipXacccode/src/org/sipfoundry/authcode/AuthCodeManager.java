/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
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
