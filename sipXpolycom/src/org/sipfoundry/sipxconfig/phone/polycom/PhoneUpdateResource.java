/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.Calendar;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.Context;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * REST service to update Polycom phone with correct firmware version in Postgres DB. This service
 * is called by the provision servlet. Phones will be restarted after 1 minute. See wiki for more
 * details.
 */
public class PhoneUpdateResource extends Resource {
    private static final Log LOG = LogFactory.getLog(PhoneUpdateResource.class);
    private static final String EMPTY = "empty";

    private PhoneContext m_phoneContext;
    private ProfileManager m_profileManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(MediaType.ALL));
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        String serialNumber = (String) getRequest().getAttributes().get("mac");
        String version = (String) getRequest().getAttributes().get("version");
        String model = (String) getRequest().getAttributes().get("model");
        if (serialNumber == null || version == null || model == null) {
            getResponse().setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            return new StringRepresentation(EMPTY);
        }
        LOG.info(String.format("Trying to updating phone %s to version %s...", serialNumber, version));
        Phone phone = m_phoneContext.loadPhone((m_phoneContext.getPhoneIdBySerialNumber(serialNumber)));
        if (phone == null) {
            getResponse().setStatus(Status.CLIENT_ERROR_NOT_FOUND);
            return new StringRepresentation(EMPTY);
        }
        if (!(phone instanceof PolycomPhone)) {
            getResponse().setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            return new StringRepresentation(EMPTY);
        }
        DeviceVersion deviceVersion = PolycomModel.getPhoneDeviceVersion(version);
        if (!phone.getDeviceVersion().equals(deviceVersion) || !StringUtils.equals(phone.getModelId(), model)) {
            phone.setDeviceVersion(deviceVersion);
            phone.setModelId(model);
            m_phoneContext.storePhone(phone);
            Calendar c = Calendar.getInstance();
            c.roll(Calendar.MINUTE, 1);
            m_profileManager.generateProfile(phone.getId(), true, c.getTime());
            LOG.info(String.format("Updated phone ID: %d. It will be rebooted in 1 "
                    + "minute from now in order to pick up correct config.", phone.getId()));
        }
        LOG.info("Phone not updated - no change.");
        return new StringRepresentation(EMPTY);
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setProfileManager(ProfileManager profileManager) {
        m_profileManager = profileManager;
    }
}
