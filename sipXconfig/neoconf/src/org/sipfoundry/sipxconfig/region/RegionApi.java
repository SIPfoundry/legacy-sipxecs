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
package org.sipfoundry.sipxconfig.region;

import static org.restlet.data.MediaType.APPLICATION_JSON;

import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.type.TypeReference;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public class RegionApi extends Resource {
    private RegionManager m_regionManager;
    private ObjectMapper m_jsonMapper = new ObjectMapper();
    private Integer m_regionId;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
        String id = (String) getRequest().getAttributes().get("id");
        if (id != null) {
            m_regionId = Integer.valueOf(id);
        }
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowPost() {
        return true;
    }

    @Override
    public boolean allowDelete() {
        return true;
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }

    // GET : list or specific region
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        getResponse().setStatus(Status.SUCCESS_OK);
        StringWriter json = new StringWriter();
        try {
            Object o;
            if (m_regionId != null) {
                o = m_regionManager.getRegion(m_regionId);
            } else {
                o = m_regionManager.getRegions();
            }
            m_jsonMapper.writeValue(json, o);
        } catch (Exception e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
        return new StringRepresentation(json.toString());
    }

    // POST
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        String json;
        try {
            json = IOUtils.toString(entity.getStream());
            Region r = m_jsonMapper.readValue(json, new TypeReference<Region>() { });
            r.setUniqueId(BeanWithId.UNSAVED_ID);
            m_regionManager.saveRegion(r);
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    };

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        String json;
        try {
            json = IOUtils.toString(entity.getStream());
            Region r = m_jsonMapper.readValue(json, new TypeReference<Region>() { });
            m_regionManager.saveRegion(r);
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    }

    // DELETE
    @Override
    public void removeRepresentations() throws ResourceException {
        Region r = m_regionManager.getRegion(m_regionId);
        m_regionManager.deleteRegion(r);
    }
}
