/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.List;

import com.mongodb.DBObject;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;

import static org.apache.commons.lang.StringUtils.defaultString;
import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class Credentials extends DataSetGenerator {
    public static final String REALM = "rlm";
    public static final String PASSTOKEN = "pstk";
    public static final String PINTOKEN = "pntk";
    public static final String AUTHTYPE = "authtp";
    public static final String DIGEST = "DIGEST";
    private CallGroupContext m_callGroupContext;

    @Override
    protected DataSet getType() {
        return DataSet.CREDENTIAL;
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }

    @Override
    public void generate() {
        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                generate(user);
            }
        };
        forAllUsersDo(getCoreContext(), closure);

        List<InternalUser> internalUsers = getCoreContext().loadInternalUsers();
        for (InternalUser user : internalUsers) {
            generate(user);
        }

        for (SpecialUserType specialUserType : SpecialUserType.values()) {
            SpecialUser user = getCoreContext().getSpecialUserAsSpecialUser(specialUserType);
            if (user != null) {
                generate(user);
            }
        }
        List<CallGroup> callGroups = m_callGroupContext.getCallGroups();
        for (CallGroup callGroup : callGroups) {
            if (callGroup.isEnabled()) {
                generate(callGroup);
            }
        }
    }

    @Override
    public void generate(Replicable entity) {
        DBObject top = findOrCreate(entity);
        String realm = getCoreContext().getAuthorizationRealm();
        if (entity instanceof User) {
            User user = (User) entity;
            insertCredential(top, realm, defaultString(user.getSipPassword()), user.getPintoken(), DIGEST);
        } else if (entity instanceof InternalUser) {
            InternalUser user = (InternalUser) entity;
            insertCredential(top, realm, defaultString(user.getSipPassword()), user.getPintoken(), DIGEST);
        } else if (entity instanceof CallGroup) {
            CallGroup callGroup = (CallGroup) entity;
            insertCredential(top, realm, callGroup.getSipPassword(), callGroup.getSipPasswordHash(realm), DIGEST);
        } else if (entity instanceof SpecialUser) {
            SpecialUser user = (SpecialUser) entity;
            insertCredential(top, realm, defaultString(user.getSipPassword()), null, DIGEST);
        } else if (entity instanceof BeanWithUserPermissions) {
            InternalUser user = ((BeanWithUserPermissions) entity).getInternalUser();
            insertCredential(top, realm, defaultString(user.getSipPassword()), user.getPintoken(), DIGEST);
        }
    }

    private void insertCredential(DBObject top, String realm, String passtoken, String pintoken, String authtype) {
        top.put(REALM, realm);
        top.put(PASSTOKEN, passtoken);
        if (!StringUtils.isEmpty(pintoken)) {
            top.put(PINTOKEN, pintoken);
        }
        top.put(AUTHTYPE, authtype);
        getDbCollection().save(top);
    }
}
