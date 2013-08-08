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
package org.sipfoundry.sipxconfig.mongo;

import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.TreeMap;

import com.mongodb.util.JSON;

public class MongoMeta {
    private Map<String, MongoNode> m_nodes;
    private Map<String, Map<String, Object>> m_meta;
    private List<String> m_clusterStatus;
    private Map<String, Map<String, List<String>>> m_analysis;

    @SuppressWarnings("unchecked")
    public void setStatusToken(String statusToken) {
        Map<String, Object> statusData = (Map<String, Object>) JSON.parse(statusToken);
        m_clusterStatus = (List<String>) statusData.get("cluster");
        Map<String, List<String>> states = (Map<String, List<String>>) statusData.get("states");
        m_nodes = new TreeMap<String, MongoNode>();
        for (Entry<String, List<String>> entry : states.entrySet()) {
            MongoNode node = new MongoNode(entry.getKey(), entry.getValue());
            m_nodes.put(entry.getKey(), node);
        }
        m_meta = (Map<String, Map<String, Object>>) statusData.get("meta");
    }
    
    /**
     * Used when a mongo cluster has no nodes yet
     */
    public void setEmptyStatus() {
        m_nodes = new TreeMap<String, MongoNode>();    	
    }

    @SuppressWarnings("unchecked")
    public void setAnalysisToken(String analysisToken) {
        m_analysis = (Map<String, Map<String, List<String>>>) JSON
                .parse(analysisToken);
    }

    public MongoNode getNode(String id) {
        return m_nodes.get(id);
    }

    public Map<String, Object> getMetaData(String server) {
        return m_meta.get(server);
    }

    public Collection<String> getServers() {
        return m_nodes.keySet();
    }

    public Collection<MongoNode> getNodes() {
        return m_nodes.values();
    }

    public List<String> getClusterStatus() {
        getServers();
        return m_clusterStatus;
    }

    public Collection<String> getRequiredActions(String server) {
        return getActions("required", server);
    }

    public Collection<String> getAvailableActions(String server) {
        return getActions("available", server);
    }

    public Collection<String> getActions(String type, String server) {
        Map<String, List<String>> actionsByType = (Map<String, List<String>>) m_analysis.get(type);
        List<String> actions = actionsByType.get(server);
        if (actions == null) {
            actions = Collections.emptyList();
        }
        return actions;
    }

    public MongoNode getPrimary() {
        for (Map.Entry<String, MongoNode> node : m_nodes.entrySet()) {
            if (node.getValue().getStatus().contains("PRIMARY")) {
                return node.getValue();
            }
        }
        return null;
    }
}
