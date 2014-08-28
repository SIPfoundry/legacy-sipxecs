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

import org.restlet.resource.Representation;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;

public class PersonalAttendantLangResource extends UserResource {
    private LocalizationContext m_context;

    @Override
    public boolean allowPost() {
        return false;
    }

    @Override
    public boolean allowDelete() {
        return false;
    }

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        String[] langs = m_context.getInstalledLanguages();
        return toRepresentation(langs);
    }

    public void setLocalizationContext(LocalizationContext context) {
        m_context = context;
    }

}
