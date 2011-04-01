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

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;

public class Permissions extends DataSetGenerator {
    public static final String PERMISSIONS = "prm";

    private User addSpecialUser(String userId) {
        User user = getCoreContext().newUser();
        user.setPermission(PermissionName.VOICEMAIL, false);
        user.setPermission(PermissionName.FREESWITH_VOICEMAIL, false);
        user.setPermission(PermissionName.EXCHANGE_VOICEMAIL, false);
        user.setPermission(PermissionName.TUI_CHANGE_PIN, false);
        user.setPermission(PermissionName.MUSIC_ON_HOLD, false);
        user.setPermission(PermissionName.PERSONAL_AUTO_ATTENDANT, false);
        user.setPermission(PermissionName.SUBSCRIBE_TO_PRESENCE, false);
        user.setUserName(userId);
        return user;
    }

    @Override
    protected DataSet getType() {
        return DataSet.PERMISSION;
    }

    @Override
    public void generate(Replicable entity) {
        if (entity instanceof User) {
            User user = (User) entity;
            insertDbObject(user);
        } else if (entity instanceof CallGroup) {
            CallGroup callGroup = (CallGroup) entity;
            if (!callGroup.isEnabled()) {
                return;
            }
            // HACK: set the user name as what we'd like to have in the id field of mongo object
            User user = addSpecialUser(CallGroup.class.getSimpleName() + callGroup.getId());
            user.setIdentity(callGroup.getIdentity(getSipDomain()));
            insertDbObject(user);
        } else if (entity instanceof SpecialUser) {
            SpecialUser specialUser = (SpecialUser) entity;
            if (specialUser.getType().equals(SpecialUserType.PHONE_PROVISION.toString())) {
                return;
            }
            User u = addSpecialUser(specialUser.getUserName());
            u.setIdentity(null);
            u.setValidUser(false);
            insertDbObject(u);
        } else if (entity instanceof BeanWithUserPermissions) {
            InternalUser user = ((BeanWithUserPermissions) entity).getInternalUser();
            User u = addSpecialUser(user.getUserName());
            u.setUserName(entity.getClass().getSimpleName() + ((BeanWithUserPermissions) entity).getId());
            insertDbObject(u);
        }
    }

    private void insertDbObject(User user) {
        DBObject top = findOrCreate(user);
        top.put(PERMISSIONS, user.getPermissions());
        getDbCollection().save(top);
    }

}
