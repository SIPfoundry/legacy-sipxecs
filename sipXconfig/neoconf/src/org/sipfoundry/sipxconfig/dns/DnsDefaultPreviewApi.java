/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.dns;

import static org.restlet.data.MediaType.APPLICATION_JSON;

import java.io.IOException;

import org.codehaus.jackson.JsonParseException;
import org.codehaus.jackson.map.JsonMappingException;
import org.codehaus.jackson.map.ObjectMapper;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;

public class DnsDefaultPreviewApi extends Resource {
    private DnsPreview m_dnsPreview;
    private DnsManager m_dnsManager;
    private final ObjectMapper m_jsonMapper = new ObjectMapper();

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public boolean allowPost() {
        return true;
    };

    // POST
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        DnsView view = m_dnsManager.getDefaultView();
        try {
            //parse request to get the custom records
            DnsView viewFromRequest = m_jsonMapper.readValue(entity.getStream(), DnsView.class);
            view.setCustomRecordsIds(viewFromRequest.getCustomRecordsIds());
        } catch (JsonParseException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        } catch (JsonMappingException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
        getResponse().setEntity(new StringRepresentation(m_dnsPreview.getZone(view, DnsPreview.Show.ALL)));
    }

    public void setDnsPreview(DnsPreview dnsPreview) {
        m_dnsPreview = dnsPreview;
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }

}
