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

public class ImSettingsResource extends UserResource {
    private static final Log LOG = LogFactory.getLog(ImSettingsResource.class);

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
        ImSettingsBean settings = new ImSettingsBean();
        User user = getUser();

        settings.setStatusPhonePresence((Boolean) user.getSettingTypedValue(AbstractUser.ADVERTISE_SIP_PRESENCE));
        settings.setStatusCallInfo((Boolean) user.getSettingTypedValue(AbstractUser.INCLUDE_CALL_INFO));
        settings.setOtpMessage((String) user.getSettingTypedValue(AbstractUser.ON_THE_PHONE_MESSAGE));
        settings.setVoicemailOnDnd((Boolean) user.getSettingTypedValue(AbstractUser.FWD_TO_VM_ON_DND));

        LOG.debug("Returning IM prefs:\t" + settings);

        return toRepresentation(settings);
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        ImSettingsBean settings = fromRepresentation(entity, ImSettingsBean.class);

        Boolean statusPhonePresence = settings.getStatusPhonePresence();
        Boolean statusCallInfo = settings.getStatusCallInfo();
        String otpMessage = settings.getOtpMessage();
        Boolean voicemailOnDnd = settings.getVoicemailOnDnd();

        LOG.debug("Saving IM prefs:\t" + settings);

        if (statusPhonePresence != null || statusCallInfo != null || otpMessage != null || voicemailOnDnd != null) {
            User user = getUser();
            if (statusPhonePresence != null) {
                user.setSettingTypedValue(AbstractUser.ADVERTISE_SIP_PRESENCE, statusPhonePresence);
            }
            if (statusCallInfo != null) {
                user.setSettingTypedValue(AbstractUser.INCLUDE_CALL_INFO, statusCallInfo);
            }
            if (otpMessage != null) {
                user.setSettingTypedValue(AbstractUser.ON_THE_PHONE_MESSAGE, otpMessage);
            }
            if (voicemailOnDnd != null) {
                user.setSettingTypedValue(AbstractUser.FWD_TO_VM_ON_DND, voicemailOnDnd);
            }
            getCoreContext().saveUser(user);
        }
    }

    // the JSON representation of this is sent to/from the client
    private static class ImSettingsBean {
        private Boolean m_statusPhonePresence;
        private Boolean m_statusCallInfo;
        private String m_otpMessage;
        private Boolean m_voicemailOnDnd;

        public Boolean getStatusPhonePresence() {
            return m_statusPhonePresence;
        }

        public void setStatusPhonePresence(Boolean statusPhonePresence) {
            m_statusPhonePresence = statusPhonePresence;
        }

        public Boolean getStatusCallInfo() {
            return m_statusCallInfo;
        }

        public void setStatusCallInfo(Boolean statusCallInfo) {
            m_statusCallInfo = statusCallInfo;
        }

        public String getOtpMessage() {
            return m_otpMessage;
        }

        public void setOtpMessage(String otpMessage) {
            m_otpMessage = otpMessage;
        }

        public Boolean getVoicemailOnDnd() {
            return m_voicemailOnDnd;
        }

        public void setVoicemailOnDnd(Boolean voicemailOnDnd) {
            m_voicemailOnDnd = voicemailOnDnd;
        }

        @Override
        public String toString() {
            return "ImSettingsBean [m_statusPhonePresence=" + m_statusPhonePresence + ", m_statusCallInfo="
                + m_statusCallInfo + ", m_otpMessage=" + m_otpMessage + ", m_voicemailOnDnd=" + m_voicemailOnDnd
                + "]";
        }
    }
}
