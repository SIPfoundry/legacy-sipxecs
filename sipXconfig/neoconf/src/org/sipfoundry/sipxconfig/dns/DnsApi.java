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

import static java.lang.Integer.parseInt;
import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.sipfoundry.sipxconfig.common.DataCollectionUtil.idAsStringToNameMap;

import java.io.IOException;
import java.io.InputStream;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.codehaus.jackson.JsonFactory;
import org.codehaus.jackson.JsonNode;
import org.codehaus.jackson.JsonParseException;
import org.codehaus.jackson.JsonParser;
import org.codehaus.jackson.JsonToken;
import org.codehaus.jackson.map.ObjectMapper;
import org.codehaus.jackson.node.ArrayNode;
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
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.region.Region;
import org.sipfoundry.sipxconfig.region.RegionManager;

public class DnsApi extends Resource {
    private static final String ID = "id";
    private static final Integer BLANK_ID = BeanWithId.UNSAVED_ID;
    private Integer m_planId;
    private ObjectMapper m_jsonMapper = new ObjectMapper();
    private DnsManager m_dnsManager;
    private RegionManager m_regionManager;
    private LocationsManager m_locationsManager;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(APPLICATION_JSON));
        String id = (String) getRequest().getAttributes().get(ID);
        if (id != null) {
            if (id.equals("blank")) {
                m_planId = BLANK_ID;
            } else {
                m_planId = Integer.valueOf(id);
            }
        }
    }

    @Override
    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowPost() {
        return true;
    };

    @Override
    public boolean allowDelete() {
        return true;
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
            if (m_planId != null) {
                json.append("{\"targetCandidates\":");
                List<Region> regions = m_regionManager.getRegions();
                List<Location> locations = m_locationsManager.getLocationsList();
                writeTargetCandidates(json, regions, locations);
                json.append(",\"plan\":");
                DnsFailoverPlan plan;
                if (m_planId == BLANK_ID) {
                    plan = createBlankPlan();
                } else {
                    plan = m_dnsManager.getPlan(m_planId);
                }
                writePlan(json, plan);
                json.append("}");
            } else {
                Collection<DnsView> views = m_dnsManager.getViews();
                Collection<DnsFailoverPlan> plans = m_dnsManager.getPlans();
                getPlans(json, plans, views);
            }
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
        return new StringRepresentation(json.toString());
    }

    DnsFailoverPlan createBlankPlan() {
        DnsFailoverPlan plan = new DnsFailoverPlan();
        DnsFailoverGroup local = new DnsFailoverGroup();
        DnsTarget localTarget = new DnsTarget(DnsTarget.BasicType.LOCAL_REGION);
        localTarget.setPercentage(100);
        local.setTargets(Collections.singletonList(localTarget));
        DnsFailoverGroup other = new DnsFailoverGroup();
        DnsTarget otherTarget = new DnsTarget(DnsTarget.BasicType.ALL_OTHER_REGIONS);
        otherTarget.setPercentage(100);
        other.setTargets(Collections.singletonList(otherTarget));
        plan.setGroups(Arrays.asList(local, other));
        return plan;
    }

    // POST
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        DnsFailoverPlan plan = readPlanHandleErrors(entity);
        plan.setUniqueId(BeanWithId.UNSAVED_ID);
        m_dnsManager.savePlan(plan);

        // caller needs the object id so it can follow-up calls
        getResponse().setEntity(new StringRepresentation(plan.getId().toString()));
    }

    // PUT
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        DnsFailoverPlan plan = readPlanHandleErrors(entity);
        m_dnsManager.savePlan(plan);
    }

    DnsFailoverPlan readPlanHandleErrors(Representation entity) throws ResourceException {
        try {
            return readPlan(entity.getStream(), m_regionManager.getRegions(), m_locationsManager.getLocationsList());
        } catch (IllegalAccessException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        } catch (InvocationTargetException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
    }

    DnsFailoverPlan readPlan(InputStream in, List<Region> regions, List<Location> locations)
        throws IOException, IllegalAccessException, InvocationTargetException {
        JsonFactory factory = new JsonFactory();
        JsonParser parser = factory.createJsonParser(in);
        parser.nextToken();
        DnsFailoverPlan plan = new DnsFailoverPlan();
        Map<Integer, Region> regionMap = DataCollectionUtil.idToObjectMap(regions);
        Map<Integer, Location> locationMap = DataCollectionUtil.idToObjectMap(locations);
        while (parser.nextToken() != JsonToken.END_OBJECT) {
            String prop = parser.getCurrentName();
            parser.nextToken();
            if (prop.equals("groups")) {
                List<DnsFailoverGroup> groups = new ArrayList<DnsFailoverGroup>();
                plan.setGroups(groups);
                while (parser.nextToken() != JsonToken.END_ARRAY) {
                    groups.add(readGroup(parser, regionMap, locationMap));
                }
            } else if (prop.equals(ID)) {
                plan.setUniqueId(parser.getIntValue());
            } else if (prop.equals("name")) {
                plan.setName(parser.getText());
            } else {
                throw newUnhandledProperty(prop, parser);
            }
        }

        return plan;
    }

    JsonParseException newUnhandledProperty(String prop, JsonParser parser) {
        return new JsonParseException("Unhandled property " + prop, parser.getCurrentLocation());
    }

    DnsFailoverGroup readGroup(JsonParser parser, Map<Integer, Region> regions, Map<Integer, Location> locations)
        throws JsonParseException, IOException, IllegalAccessException, InvocationTargetException {
        DnsFailoverGroup group = new DnsFailoverGroup();
        while (parser.nextToken() != JsonToken.END_OBJECT) {
            String prop = parser.getCurrentName();
            parser.nextToken();
            if (prop.equals("targets")) {
                List<DnsTarget> targets = new ArrayList<DnsTarget>();
                group.setTargets(targets);
                while (parser.nextToken() != JsonToken.END_ARRAY) {
                    targets.add(readTarget(parser, regions, locations));
                }
            } else if (prop.equals(ID)) {
                group.setUniqueId(parser.getIntValue());
            } else {
                throw newUnhandledProperty(prop, parser);
            }
        }
        return group;
    }

    DnsTarget readTarget(JsonParser parser, Map<Integer, Region> regions, Map<Integer, Location> locations)
        throws JsonParseException, IOException {
        int percentage = 0;
        String targetIdStr = null;
        DnsTarget.Type type = null;
        while (parser.nextToken() != JsonToken.END_OBJECT) {
            String prop = parser.getCurrentName();
            parser.nextToken();
            if (prop.equals("targetType")) {
                type = DnsTarget.Type.valueOf(parser.getText());
            } else if (prop.equals("percentage")) {
                percentage = parser.getIntValue();
            } else if (prop.equals("targetId")) {
                targetIdStr = parser.getText();
            } else {
                throw newUnhandledProperty(prop, parser);
            }
        }
        DnsTarget target = null;
        switch (type) {
        case BASIC:
            DnsTarget.BasicType basicType = DnsTarget.BasicType.valueOf(targetIdStr);
            target = new DnsTarget(basicType);
            break;
        case LOCATION:
            Integer locationId = parseInt(targetIdStr);
            target = new DnsTarget(locations.get(locationId));
            break;
        case REGION:
            Integer regionId = parseInt(targetIdStr);
            target = new DnsTarget(regions.get(regionId));
            break;
        default:
            throw new IllegalStateException();
        }
        target.setPercentage(percentage);
        return target;
    }

    // DELETE
    @Override
    public void removeRepresentations() throws ResourceException {
        DnsFailoverPlan plan = m_dnsManager.getPlan(m_planId);
        m_dnsManager.deletePlan(plan);
    }

    void writePlan(Writer json, DnsFailoverPlan plan) throws IOException {
        m_jsonMapper.writeValue(json, plan);
    }

    void getPlans(Writer json, Collection<DnsFailoverPlan> plans, Collection<DnsView> views) throws IOException {
        Map<Integer, List<String>> viewsNamesByPlanId = new HashMap<Integer, List<String>>();
        for (DnsView view : views) {
            if (view.getPlanId() != null) {
                List<String> viewsForPlan = viewsNamesByPlanId.get(view.getPlanId());
                if (viewsForPlan == null) {
                    viewsForPlan = new ArrayList<String>();
                    viewsNamesByPlanId.put(view.getPlanId(), viewsForPlan);
                }
                viewsForPlan.add(view.getName());
            }
        }
        StringWriter temp = new StringWriter();
        m_jsonMapper.writeValue(temp, plans);
        JsonNode objs = m_jsonMapper.reader().readTree(new StringReader(temp.toString()));
        Iterator<JsonNode> elements = objs.getElements();
        while (elements.hasNext()) {
            ObjectNode plan = (ObjectNode) elements.next();
            List<String> viewsForPlan = viewsNamesByPlanId.get(plan.get(ID).asInt());
            if (viewsForPlan != null) {
                ArrayNode a = plan.putArray("views");
                for (String name : viewsForPlan) {
                    a.add(name);
                }
            }
        }

        m_jsonMapper.writeValue(json, objs);
    }

    void writeTargetCandidates(Writer json, Collection<Region> regions, Collection<Location> locations)
        throws IOException {
        Map<String, Object> candidates = new TreeMap<String, Object>();
        for (DnsTarget.BasicType type : DnsTarget.BasicType.values()) {
            candidates.put(type.name(), type);
        }
        candidates.put(DnsTarget.Type.REGION.name(), idAsStringToNameMap(regions));
        candidates.put(DnsTarget.Type.LOCATION.name(), locationsToMap(locations));
        m_jsonMapper.writeValue(json, candidates);
    }

    Map<String, Object> locationsToMap(Collection<Location> locations) {
        Map<String, Object> m = new HashMap<String, Object>();
        for (Location l : locations) {
            m.put(l.getId().toString(), l.getHostnameInSipDomain());
        }
        return m;
    }

    public void setDnsManager(DnsManager dnsManager) {
        m_dnsManager = dnsManager;
    }

    public void setRegionManager(RegionManager regionManager) {
        m_regionManager = regionManager;
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }
}
