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
import static org.sipfoundry.sipxconfig.common.DataCollectionUtil.idToObjectMap;

import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;
import java.util.Collection;
import java.util.Iterator;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.JsonParseException;
import org.codehaus.jackson.map.JsonMappingException;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.node.ObjectNode;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.cfgmgt.JsonConfigurationFile;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.region.Region;

public class DnsDefaultViewApi extends Resource {
    private static final Log LOG = LogFactory.getLog(DnsDefaultViewApi.class);
    private final ObjectMapper m_jsonMapper = new ObjectMapper();
    private DnsManager m_dnsManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowPost() {
        return false;
    };

    @Override
    public boolean allowDelete() {
        return false;
    };

    @Override
    public boolean allowPut() {
        return true;
    };

    ObjectMapper getJsonMapper() {
        return m_jsonMapper;
    }

    // GET
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        StringWriter json = new StringWriter();
        try {
            DnsView view = m_dnsManager.getDefaultView();
            LOG.debug("View: " + view);
            writeView(json, view, m_dnsManager.getCustomRecords());
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
        return new StringRepresentation(json.toString());
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        DnsView view = m_dnsManager.getDefaultView();
        DnsView viewFromRequest = readViewHandleErrors(entity);
        view.setCustomRecordsIds(viewFromRequest.getCustomRecordsIds());
        view.setPlanId(null);
        view.setRegionId(null);
        m_dnsManager.saveView(view);
    }

    DnsView readViewHandleErrors(Representation entity) throws ResourceException {
        try {
            return readView(entity.getStream());
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    }

    DnsView readView(InputStream in) throws JsonParseException, JsonMappingException, IOException {
        return getJsonMapper().readValue(in, DnsView.class);
    }

    void writeViews(Writer json, Collection<DnsView> views, Collection<Region> regionList,
            Collection<DnsFailoverPlan> planList) throws IOException {
        Map<Integer, DnsFailoverPlan> plans = idToObjectMap(planList);
        Map<Integer, Region> regions = idToObjectMap(regionList);
        StringWriter temp = new StringWriter();
        m_jsonMapper.writeValue(temp, views);
        JsonNode objs = m_jsonMapper.reader().readTree(new StringReader(temp.toString()));
        Iterator<JsonNode> elements = objs.getElements();
        while (elements.hasNext()) {
            ObjectNode view = (ObjectNode) elements.next();
            JsonNode regionId = view.get("regionId");
            if (regionId != null) {
                view.put("region", regions.get(regionId.asInt()).getName());
            }
            JsonNode planIdNode = view.get("planId");
            if (planIdNode != null) {
                int planId = planIdNode.asInt();
                if (planId > 0) {
                    view.put("plan", plans.get(planId).getName());
                }
            }
        }
        getJsonMapper().writeValue(json, objs);
    }

    void writeView(Writer json, DnsView view, Collection<DnsCustomRecords> customRecords) throws IOException {
        JsonConfigurationFile f = new JsonConfigurationFile(json);
        f.open("customRecordsCandidates");
        getJsonMapper().writeValue(json, DataCollectionUtil.idToNameMap(customRecords));
        f.close();
        f.open("view");
        getJsonMapper().writeValue(json, view);
        f.close();
        f.close();
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }

}
