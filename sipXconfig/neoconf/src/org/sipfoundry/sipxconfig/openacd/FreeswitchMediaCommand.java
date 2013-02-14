/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.openacd;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;

public class FreeswitchMediaCommand extends BeanWithId implements Replicable {

    private boolean m_enabled;
    private String m_node;
    private String m_dialString;

    public FreeswitchMediaCommand(boolean enabled, String node, String dialString) {
        m_enabled = enabled;
        m_node = node;
        m_dialString = dialString;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public void setNode(String node) {
        m_node = node;
    }

    public String getNode() {
        return m_node;
    }

    public void setDialString(String dialString) {
        m_dialString = dialString;
    }

    public String getDialString() {
        return m_dialString;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        props.put(MongoConstants.ENABLED, isEnabled());
        props.put(OpenAcdContext.NODE, getNode());
        props.put(OpenAcdContext.DIAL_STRING, getDialString());
        return props;
    }

    @Override
    public Set<DataSet> getDataSets() {
        return Collections.singleton(DataSet.OPENACD);
    }

    @Override
    public String getIdentity(String domainName) {
        return null;
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        return null;
    }

    @Override
    public boolean isValidUser() {
        return false;
    }

    @Override
    public String getName() {
        return null;
    }

    @Override
    public void setName(String name) {
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }
}
