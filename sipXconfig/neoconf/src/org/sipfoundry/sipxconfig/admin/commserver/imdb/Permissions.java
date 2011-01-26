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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.admin.authcode.AuthCode;
import org.sipfoundry.sipxconfig.admin.authcode.AuthCodeManager;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeer;
import org.sipfoundry.sipxconfig.admin.tls.TlsPeerManager;
import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class Permissions extends DataSetGenerator {
    public static final String PERMISSIONS = "prm";
    private CallGroupContext m_callGroupContext;
    private TlsPeerManager m_tlsPeerManager;
    private AuthCodeManager m_authCodeManager;

    void addUser(List<DataSetRecord> records, User user) {
        Setting permissions = user.getSettings().getSetting(Permission.CALL_PERMISSION_PATH);
        Setting voicemailPermissions = user.getSettings().getSetting(Permission.VOICEMAIL_SERVER_PATH);
        PermissionWriter writer = new PermissionWriter(user, records);
        permissions.acceptVisitor(writer);
        voicemailPermissions.acceptVisitor(writer);
    }

    private User addSpecialUser(String userId, List<DataSetRecord> records) {
        User user = getCoreContext().newUser();
        user.setPermission(PermissionName.VOICEMAIL, false);
        user.setPermission(PermissionName.FREESWITH_VOICEMAIL, false);
        user.setPermission(PermissionName.EXCHANGE_VOICEMAIL, false);
        user.setUserName(userId);
        addUser(records, user);
        return user;
    }

    class PermissionWriter extends AbstractSettingVisitor {
        private final User m_user;
        private final List<DataSetRecord> m_records;

        PermissionWriter(User user, List<DataSetRecord> records) {
            m_user = user;
            m_records = records;
        }

        @Override
        public void visitSetting(Setting setting) {
            if (!Permission.isEnabled(setting.getValue())) {
                return;
            }
            String name = setting.getName();
            PermissionMapping mapping = new PermissionMapping(name);
            mapping.setEntity(m_user);
            m_records.add(mapping);
        }
    }

    @Override
    protected DataSet getType() {
        return DataSet.PERMISSION;
    }

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }

    @Override
    public void generate(Replicable entity) {
        List<DataSetRecord> records = new ArrayList<DataSetRecord>();
        if (entity instanceof User) {
            User user = (User) entity;
            addUser(records, user);
            insertDbObject(user, records);
        } else if (entity instanceof CallGroup) {
            CallGroup callGroup = (CallGroup) entity;
            // HACK: set the user name as what we'd like to have in the id field of mongo object
            User user = addSpecialUser(CallGroup.class.getSimpleName() + callGroup.getId(), records);
            user.setIdentity(callGroup.getIdentity(getSipDomain()));
            insertDbObject(user, records);
        } else if (entity instanceof SpecialUser) {
            SpecialUser specialUser = (SpecialUser) entity;
            User u = addSpecialUser(specialUser.getUserName(), records);
            u.setIdentity(null);
            insertDbObject(u, records);
        } else if (entity instanceof BeanWithUserPermissions) {
            InternalUser user = ((BeanWithUserPermissions) entity).getInternalUser();
            User u = addSpecialUser(user.getUserName(), records);
            u.setUserName(entity.getClass().getSimpleName() + ((BeanWithUserPermissions) entity).getId());
            insertDbObject(u, records);
        }
    }

    @Override
    public void generate() {
        for (SpecialUserType sut : SpecialUserType.values()) {

            // As PHONE_PROVISION does NOT require any permissions, skip it.
            if (!sut.equals(SpecialUserType.PHONE_PROVISION)) {
                SpecialUser sp = getCoreContext().getSpecialUserAsSpecialUser(sut);
                generate(sp);
            }
        }

        List<CallGroup> callGroups = m_callGroupContext.getCallGroups();
        for (CallGroup callGroup : callGroups) {
            if (callGroup.isEnabled()) {
                generate(callGroup);
            }
        }

        List<TlsPeer> tlsPeers = m_tlsPeerManager.getTlsPeers();
        for (TlsPeer tlsPeer : tlsPeers) {
            generate(tlsPeer);
        }

        List<AuthCode> authCodes = m_authCodeManager.getAuthCodes();
        for (AuthCode authCode : authCodes) {
            generate(authCode);
        }

        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                generate(user);
            }
        };
        forAllUsersDo(getCoreContext(), closure);

    }

    private void insertDbObject(Replicable entity, List<DataSetRecord> records) {
        DBObject top = findOrCreate(entity);
        Collection<String> prms = new ArrayList<String>();
        for (DataSetRecord record : records) {
            prms.add(record.get(PermissionMapping.PERMISSION).toString());
        }
        top.put(PERMISSIONS, prms);
        getDbCollection().save(top);
    }

    public void setAuthCodeManager(AuthCodeManager authCodeManager) {
        m_authCodeManager = authCodeManager;
    }

    public void setTlsPeerManager(TlsPeerManager tlsPeerManager) {
        m_tlsPeerManager = tlsPeerManager;
    }

}
