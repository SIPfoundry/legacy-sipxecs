/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.rest;

import static org.sipfoundry.sipxconfig.rest.JacksonConvert.fromRepresentation;
import static org.sipfoundry.sipxconfig.rest.JacksonConvert.toRepresentation;

import java.util.LinkedHashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendantManager;

public class PersonalAttendantResource extends UserResource {
    private static final Log LOG = LogFactory.getLog(PersonalAttendantResource.class);

    private PersonalAttendantManager m_mgr;

    @Override
    public boolean allowPost() {
        return false;
    }

    @Override
    public boolean allowDelete() {
        return false;
    }

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        AttendantBean settings = new AttendantBean();
        User user = getUser();

        settings.setPersonalAttendantPermission(user.hasPermission(PermissionName.PERSONAL_AUTO_ATTENDANT));
        settings.setDepositVM(user.isDepositVoicemail());
        settings.setPlayVMDefaultOptions(user.getPlayVmDefaultOptions());
        settings.setOperator((String) user.getSettingTypedValue(AbstractUser.OPERATOR_SETTING));

        PersonalAttendant att = m_mgr.loadPersonalAttendantForUser(user);

        settings.setLanguage(att.getLanguage());
        settings.setOverrideLanguage(att.getOverrideLanguage());
        Map<String, String> menuMap = new LinkedHashMap<String, String>();
        AttendantMenu menu = att.getMenu();

        if (menu != null) {
            for (Map.Entry<DialPad, AttendantMenuItem> entry : menu.getMenuItems().entrySet()) {
                menuMap.put(entry.getKey().getName(), entry.getValue().getParameter());
            }
            settings.setMenu(menuMap);
        }

        LOG.debug("Returning attendant prefs:\t" + settings);

        return toRepresentation(settings);
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        if (Boolean.TRUE == getUser().hasPermission(PermissionName.PERSONAL_AUTO_ATTENDANT)) {
            AttendantBean settings = fromRepresentation(entity, AttendantBean.class);
            LOG.debug("Got attendant prefs:\t" + settings);
            User user = getUser();

            if (settings.getDepositVM() != null) {
                user.setDepositVoicemail(settings.getDepositVM());
            }
            if (settings.getPlayVMDefaultOptions() != null) {
                user.setPlayVmDefaultOptions(settings.getPlayVMDefaultOptions());
            }
            if (settings.getOperator() != null) {
                user.setOperator(settings.getOperator());
            }

            PersonalAttendant attendant = m_mgr.loadPersonalAttendantForUser(user);
            Map<String, String> menuMap = settings.getMenu();

            if (menuMap != null) {
                AttendantMenu menu = new AttendantMenu();
                for (Map.Entry<String, String> entry : menuMap.entrySet()) {
                    DialPad dial = DialPad.getByName(entry.getKey());
                    menu.addMenuItem(dial, AttendantMenuAction.TRANSFER_OUT, entry.getValue());
                }
                attendant.setMenu(menu);
            }
            if (settings.getLanguage() != null) {
                attendant.setLanguage(settings.getLanguage());
            }
            if (settings.getLanguage() != null) {
                attendant.setOverrideLanguage(settings.getOverrideLanguage());
            }

            // try to be smart and don't save what's not present in the request
            if (menuMap != null || settings.getLanguage() != null || settings.getLanguage() != null) {
                LOG.debug("Saving attendant prefs:\t" + attendant);

                m_mgr.storePersonalAttendant(attendant);
            }
            if (settings.getDepositVM() != null || settings.getPlayVMDefaultOptions() != null
                || settings.getOperator() != null) {
                LOG.debug("Saving user");

                getCoreContext().saveUser(user);
            }
        }
    }

    public void setMgr(PersonalAttendantManager mgr) {
        m_mgr = mgr;
    }

    // the JSON representation of this is sent to/from the client
    private static class AttendantBean {
        private Boolean m_depositVM;
        private Boolean m_playVMDefaultOptions;
        private String m_operator;
        private Map<String, String> m_menu;
        private String m_language;
        private Boolean m_overrideLanguage;
        private Boolean m_personalAttendantPermission;

        public Boolean getDepositVM() {
            return m_depositVM;
        }

        public void setDepositVM(Boolean depositVM) {
            m_depositVM = depositVM;
        }

        public Boolean getPlayVMDefaultOptions() {
            return m_playVMDefaultOptions;
        }

        public void setPlayVMDefaultOptions(Boolean playVMDefaultOptions) {
            m_playVMDefaultOptions = playVMDefaultOptions;
        }

        public String getOperator() {
            return m_operator;
        }

        public void setOperator(String operator) {
            m_operator = operator;
        }

        public Map<String, String> getMenu() {
            return m_menu;
        }

        public void setMenu(Map<String, String> menu) {
            m_menu = menu;
        }

        public String getLanguage() {
            return m_language;
        }

        public void setLanguage(String language) {
            m_language = language;
        }

        public Boolean getOverrideLanguage() {
            return m_overrideLanguage;
        }

        public void setOverrideLanguage(Boolean overrideLanguage) {
            m_overrideLanguage = overrideLanguage;
        }

        @SuppressWarnings("unused")
        public Boolean getPersonalAttendantPermission() {
            return m_personalAttendantPermission;
        }

        public void setPersonalAttendantPermission(Boolean personalAttendantPermission) {
            m_personalAttendantPermission = personalAttendantPermission;
        }

        @Override
        public String toString() {
            return "AttendantBean [m_depositVM=" + m_depositVM + ", m_playVMDefaultOptions="
                + m_playVMDefaultOptions + ", m_operator=" + m_operator + ", m_menu=" + m_menu + ", m_language="
                + m_language + ", m_overrideLanguage=" + m_overrideLanguage + ", m_personalAttendantPermission="
                + m_personalAttendantPermission + "]";
        }
    }
}
