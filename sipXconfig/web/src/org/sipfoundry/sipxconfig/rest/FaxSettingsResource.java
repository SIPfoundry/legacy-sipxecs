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

import static org.sipfoundry.sipxconfig.rest.JacksonConvert.toRepresentation;

import java.util.HashMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;

public class FaxSettingsResource extends UserResource {
    private static final Log LOG = LogFactory.getLog(FaxSettingsResource.class);

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        Map<String, String> faxSettings = new HashMap<String, String>();

        faxSettings.put("extension", getUser().getFaxExtension());
        faxSettings.put("did", getUser().getFaxDid());

        LOG.debug("Returning fax prefs:\t" + faxSettings);

        return toRepresentation(faxSettings);
    }
}
