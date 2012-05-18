/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.apache.commons.lang.StringUtils.defaultString;
import static org.sipfoundry.commons.mongo.MongoConstants.AUTHTYPE;
import static org.sipfoundry.commons.mongo.MongoConstants.PASSTOKEN;
import static org.sipfoundry.commons.mongo.MongoConstants.PINTOKEN;
import static org.sipfoundry.commons.mongo.MongoConstants.REALM;
import static org.sipfoundry.commons.mongo.MongoConstants.VOICEMAIL_PINTOKEN;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.User;

import com.mongodb.DBObject;

public class Credentials extends AbstractDataSetGenerator {
    public static final String DIGEST = "DIGEST";

    @Override
    protected DataSet getType() {
        return DataSet.CREDENTIAL;
    }

    @Override
    public boolean generate(Replicable entity, DBObject top) {
        String realm = getCoreContext().getAuthorizationRealm();
        if (entity instanceof User) {
            User user = (User) entity;
            insertCredential(top, realm, defaultString(user.getSipPassword()), user.getPintoken(), DIGEST);
            insertVoicemailPin(top, user.getVoicemailPintoken());
            return true;
        } else if (entity instanceof CallGroup) {
            CallGroup callGroup = (CallGroup) entity;
            insertCredential(top, realm, callGroup.getSipPassword(), callGroup.getSipPasswordHash(realm), DIGEST);
            return true;
        } else if (entity instanceof SpecialUser) {
            SpecialUser user = (SpecialUser) entity;
            insertCredential(top, realm, defaultString(user.getSipPassword()), null, DIGEST);
            return true;
        } else if (entity instanceof BeanWithUserPermissions) {
            InternalUser user = ((BeanWithUserPermissions) entity).getInternalUser();
            insertCredential(top, realm, defaultString(user.getSipPassword()), user.getPintoken(), DIGEST);
            return true;
        }
        return false;
    }

    private void insertCredential(DBObject top, String realm, String passtoken, String pintoken, String authtype) {
        top.put(REALM, realm);
        top.put(PASSTOKEN, passtoken);
        if (!StringUtils.isEmpty(pintoken)) {
            top.put(PINTOKEN, pintoken);
        }
        top.put(AUTHTYPE, authtype);
    }

    private void insertVoicemailPin(DBObject top, String vpintoken) {
        if (!StringUtils.isEmpty(vpintoken)) {
            top.put(VOICEMAIL_PINTOKEN, vpintoken);
        }
    }

}
