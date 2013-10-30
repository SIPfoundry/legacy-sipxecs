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

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;

import static org.sipfoundry.commons.mongo.MongoConstants.PERMISSIONS;

public class Permissions extends AbstractDataSetGenerator {

    private PermissionManager m_permissionManager;

    private User addSpecialUser(String userId) {
        User user = getCoreContext().newUser();
        setSpecialUserPermissions(user);
        user.setUserName(userId);
        return user;
    }

    private User addSpecialUser(InternalUser internalUser) {
        User user = getCoreContext().newUser();
        user.setUserName(internalUser.getUserName());
        for (Permission p : m_permissionManager.getPermissions()) {
            user.setPermission(p, internalUser.hasPermission(p));
        }
        setSpecialUserPermissions(user);
        return user;
    }

    private void setSpecialUserPermissions(User user) {
        user.setPermission(PermissionName.VOICEMAIL, false);
        user.setPermission(PermissionName.FREESWITH_VOICEMAIL, false);
        user.setPermission(PermissionName.EXCHANGE_VOICEMAIL, false);
        user.setPermission(PermissionName.TUI_CHANGE_PIN, false);
        user.setPermission(PermissionName.MUSIC_ON_HOLD, false);
        user.setPermission(PermissionName.GROUP_MUSIC_ON_HOLD, false);
        user.setPermission(PermissionName.PERSONAL_AUTO_ATTENDANT, false);
        user.setPermission(PermissionName.SUBSCRIBE_TO_PRESENCE, false);
    }

    @Override
    protected DataSet getType() {
        return DataSet.PERMISSION;
    }

    @Override
    public boolean generate(Replicable entity, DBObject top) {
        if (entity instanceof User) {
            User user = (User) entity;
            insertDbObject(user, top);
            return true;
        } else if (entity instanceof CallGroup) {
            CallGroup callGroup = (CallGroup) entity;
            if (!callGroup.isEnabled()) {
                return false;
            }
            // HACK: set the user name as what we'd like to have in the id field of mongo object
            User user = addSpecialUser(CallGroup.class.getSimpleName() + callGroup.getId());
            user.setIdentity(callGroup.getIdentity(getSipDomain()));
            insertDbObject(user, top);
            return true;
        } else if (entity instanceof SpecialUser) {
            SpecialUser specialUser = (SpecialUser) entity;
            if (specialUser.getType().equals(SpecialUserType.PHONE_PROVISION.toString())) {
                return false;
            }
            User u = addSpecialUser(specialUser.getUserName());
            u.setIdentity(null);
            u.setValidUser(false);
            insertDbObject(u, top);
            return true;
        } else if (entity instanceof BeanWithUserPermissions) {
            InternalUser user = ((BeanWithUserPermissions) entity).getInternalUser();
            User u = addSpecialUser(user);
            u.setUserName(entity.getClass().getSimpleName() + ((BeanWithUserPermissions) entity).getId());
            u.setValidUser(false);
            insertDbObject(u, top);
            return true;
        }
        return false;
    }

    private void insertDbObject(User user, DBObject top) {
        top.put(PERMISSIONS, user.getPermissions());
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }
}
