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
import java.util.ArrayList;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.codehaus.jackson.JsonGenerationException;
import org.codehaus.jackson.JsonGenerator;
import org.codehaus.jackson.Version;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.map.SerializerProvider;
import org.codehaus.jackson.map.module.SimpleModule;
import org.codehaus.jackson.map.ser.std.SerializerBase;
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
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class RegionApi extends Resource {
    private static final String ID = "id";
    private RegionManager m_regionManager;
    private LocationsManager m_locationsManager;
    private ObjectMapper m_jsonMapper = new ObjectMapper();
    private Integer m_regionId;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
        String id = (String) getRequest().getAttributes().get(ID);
        if (id != null) {
            m_regionId = Integer.valueOf(id);
        }
    }

    void registerJsonSerilizer() {
        SimpleModule module = new SimpleModule("regionsWithServers", new Version(1, 0, 0, null));
        List<Location> locations = m_locationsManager.getLocationsList();
        module.addSerializer(Region.class, new RegionWithServers(locations));
        m_jsonMapper.registerModule(module);
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
    public boolean allowPut() {
        return true;
    }

    @Override
    public boolean allowDelete() {
        return true;
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    // GET : list or specific region
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        getResponse().setStatus(Status.SUCCESS_OK);
        StringWriter json = new StringWriter();
        registerJsonSerilizer();
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

    /**
     * Add server list to region as help aid to caller to know what server are in what region.
     */
    static final class RegionWithServers extends SerializerBase<Region> {
        private List<Location> m_locations;

        protected RegionWithServers(List<Location> locations) {
            super(Region.class);
            m_locations = locations;
        }

        @Override
        public void serialize(final Region region, JsonGenerator jgen, SerializerProvider provider)
            throws IOException, JsonGenerationException {

            jgen.writeStartObject();

            // Ideally, I ask default implementation to write out all the bean
            // properties so i do not have to enumerate them here. but i could
            // not figure this out -- Douglas
            jgen.writeObjectField("name", region.getName());
            jgen.writeObjectField(ID, region.getId());
            jgen.writeObjectField("addresses", region.getAddresses());

            List<String> servers = new ArrayList<String>();
            for (Location location : m_locations) {
                if (region.getId().equals(location.getRegionId())) {
                    servers.add(location.getHostname());
                }
            }
            jgen.writeObjectField("servers", servers);
            jgen.writeEndObject();
        }
    }

    // POST
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        String json;
        try {
            json = IOUtils.toString(entity.getStream());
            Region r = m_jsonMapper.readValue(json, new TypeReference<Region>() {
            });
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
            Region r = m_jsonMapper.readValue(json, new TypeReference<Region>() {
            });
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
