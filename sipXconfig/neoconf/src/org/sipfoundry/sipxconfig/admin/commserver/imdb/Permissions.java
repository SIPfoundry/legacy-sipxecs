/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Iterator;
import java.util.List;

import org.dom4j.Element;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.AbstractSettingVisitor;
import org.sipfoundry.sipxconfig.setting.Setting;

public class Permissions extends DataSetGenerator {

    /**
     * Adds <item> <identity>user_uri</identity> <permission>permission_name</permission>
     * </item>
     * 
     * to the list of items.
     */
    protected void addItems(Element items) {
        String domain = getSipDomain();
        CoreContext coreContext = getCoreContext();
        List<User> list = coreContext.loadUsers();
        for (Iterator<User> i = list.iterator(); i.hasNext();) {
            User user = i.next();
            addUser(items, user, domain);
        }
    }

    void addUser(Element item, User user, String domain) {
        Setting permissions = user.getSettings().getSetting(Permission.CALL_PERMISSION_PATH);
        Setting voicemailPermissions = 
            user.getSettings().getSetting(Permission.VOICEMAIL_SERVER_PATH);
        PermissionWriter writer = new PermissionWriter(user, domain, item);
        permissions.acceptVisitor(writer);
        voicemailPermissions.acceptVisitor(writer);
    }

    class PermissionWriter extends AbstractSettingVisitor {

        private static final String PERMISSION_ELEMENT = "permission";

        private static final String IDENTITY_ELEMENT = "identity";

        private User m_user;

        private Element m_items;

        private String m_domain;

        PermissionWriter(User user, String domain, Element items) {
            m_user = user;
            m_items = items;
            m_domain = domain;
        }

        public void visitSetting(Setting setting) {
            if (Permission.isEnabled(setting.getValue())) {
                Element userItem = addItem(m_items);
                userItem.addElement(IDENTITY_ELEMENT).setText(m_user.getUri(m_domain));
                userItem.addElement(PERMISSION_ELEMENT).setText(setting.getName());
                
                // add special permission for voicemail redirect rule for
                // xcf-1875
                if (setting.getName().equals(PermissionName.EXCHANGE_VOICEMAIL.getName())
                        || setting.getName().equals(PermissionName.SIPX_VOICEMAIL.getName())) {
                    Element voicemailRedirectItem = addItem(m_items);
                    
                    voicemailRedirectItem.addElement(IDENTITY_ELEMENT).setText(generateVoicemailRedirectIdentity());
                    voicemailRedirectItem.addElement(PERMISSION_ELEMENT).setText(setting.getName());
                }
            }
        }

        private String generateVoicemailRedirectIdentity() {
            StringBuffer voicemailRedirectNameBuffer = new StringBuffer("sip:~~vm~");
            voicemailRedirectNameBuffer.append(m_user.getName());
            voicemailRedirectNameBuffer.append('@');
            voicemailRedirectNameBuffer.append(m_domain);
            
            return voicemailRedirectNameBuffer.toString();
        }
    }

    protected DataSet getType() {
        return DataSet.PERMISSION;
    }
}
