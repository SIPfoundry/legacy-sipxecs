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

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.AbstractUser;
import org.sipfoundry.sipxconfig.common.User;

public class ImBotSettingsResource extends UserResource {
    private static final Log LOG = LogFactory.getLog(ImBotSettingsResource.class);

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
        ImBotSettingsBean settings = new ImBotSettingsBean();
        User user = getUser();

        settings.setConfEnter((Boolean) user.getSettingTypedValue(AbstractUser.NOTIFICATION_CONF_ENTERED));
        settings.setConfExit((Boolean) user.getSettingTypedValue(AbstractUser.NOTIFICATION_CONF_EXITED));
        settings.setVmBegin((Boolean) user.getSettingTypedValue(AbstractUser.NOTIFICATION_VM_BEGIN));
        settings.setVmEnd((Boolean) user.getSettingTypedValue(AbstractUser.NOTIFICATION_VM_END));

        LOG.debug("Returning IM bot prefs:\t" + settings);

        return toRepresentation(settings);
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        ImBotSettingsBean settings = fromRepresentation(entity, ImBotSettingsBean.class);

        Boolean confEnter = settings.getConfEnter();
        Boolean confExit = settings.getConfExit();
        Boolean vmBegin = settings.getVmBegin();
        Boolean vmEnd = settings.getVmEnd();

        LOG.debug("Saving IM bot prefs:\t" + settings);

        if (confEnter != null || confExit != null || vmBegin != null || vmEnd != null) {
            User user = getUser();
            if (confEnter != null) {
                user.setSettingTypedValue(AbstractUser.NOTIFICATION_CONF_ENTERED, confEnter);
            }
            if (confExit != null) {
                user.setSettingTypedValue(AbstractUser.NOTIFICATION_CONF_EXITED, confExit);
            }
            if (vmBegin != null) {
                user.setSettingTypedValue(AbstractUser.NOTIFICATION_VM_BEGIN, vmBegin);
            }
            if (vmEnd != null) {
                user.setSettingTypedValue(AbstractUser.NOTIFICATION_VM_END, vmEnd);
            }
            getCoreContext().saveUser(user);
        }
    }

    // the JSON representation of this is sent to/from the client
    private static class ImBotSettingsBean {
        private Boolean m_confEnter;
        private Boolean m_confExit;
        private Boolean m_vmBegin;
        private Boolean m_vmEnd;

        public Boolean getConfEnter() {
            return m_confEnter;
        }

        public void setConfEnter(Boolean confEnter) {
            m_confEnter = confEnter;
        }

        public Boolean getConfExit() {
            return m_confExit;
        }

        public void setConfExit(Boolean confExit) {
            m_confExit = confExit;
        }

        public Boolean getVmBegin() {
            return m_vmBegin;
        }

        public void setVmBegin(Boolean vmBegin) {
            m_vmBegin = vmBegin;
        }

        public Boolean getVmEnd() {
            return m_vmEnd;
        }

        public void setVmEnd(Boolean vmEnd) {
            m_vmEnd = vmEnd;
        }

        @Override
        public String toString() {
            return "ImBotSettingsBean [m_confEnter=" + m_confEnter + ", m_confExit=" + m_confExit + ", m_vmBegin="
                + m_vmBegin + ", m_vmEnd=" + m_vmEnd + "]";
        }
    }
}
