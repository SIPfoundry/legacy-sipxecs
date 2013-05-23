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
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.ALIAS_ID;
import static org.sipfoundry.commons.mongo.MongoConstants.CONTACT;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.digester.Digester;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.xml.sax.SAXException;

public class ExternalAlias implements Replicable {
    private static final Log LOG = LogFactory.getLog(ExternalAliases.class);
    private static final String ALIAS_IDENTITY = "identity";
    private static final String ALIAS_CONTACT = "contact";

    private List<File> m_files;

    private List<AliasMapping> parseAliases(File file) {
        try {
            // Digester documentation advises against reusing digester objects
            Digester digester = ImdbXmlHelper.configureDigester(AliasMapping.class);
            return (List<AliasMapping>) digester.parse(file);
        } catch (IOException e) {
            LOG.warn("Errors when reading aliases file.", e);
        } catch (SAXException e) {
            LOG.warn("Errors when parsing aliases file.", e);
        }
        return null;
    }

    @Override
    public String getName() {
        return "extAls";
    }

    @Override
    public void setName(String name) {
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domain) {
        List files = m_files;
        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        for (Iterator i = files.iterator(); i.hasNext();) {
            File file = (File) i.next();
            List<AliasMapping> parsedAliases = parseAliases(file);
            if (!CollectionUtils.isEmpty(parsedAliases)) {
                for (AliasMapping aliasMapping : parsedAliases) {
                    // transform AliasMappings. The resulting
                    // AliasMapping is written as {identity: "foo", contact: "bar"} and we need
                    // {id: "foo", cnt: "bar"}
                    // Also, strip the @domain part of the id if present
                    transformMapping(aliasMapping);
                    mappings.add(aliasMapping);
                }
            }
        }
        return mappings;
    }

    /**
     * Transform AliasMappings. The result from parsing aliases files is written as {identity:
     * "foo", contact: "bar"} and we need {id: "foo", cnt: "bar"} Also, strip the @domain part of
     * the id if present
     */
    private void transformMapping(AliasMapping mapping) {
        String id = mapping.getString(ALIAS_IDENTITY);
        String cnt = mapping.getString(ALIAS_CONTACT);
        if (id != null) {
            // strip domain part
            String[] tokens = StringUtils.split(id, "@");
            mapping.put(ALIAS_ID, tokens[0]);
            mapping.remove(ALIAS_IDENTITY);
        }
        if (cnt != null) {
            mapping.put(CONTACT, cnt);
            mapping.remove(ALIAS_CONTACT);
        }
    }

    @Override
    public Set<DataSet> getDataSets() {
        return Collections.singleton(DataSet.ALIAS);
    }

    @Override
    public String getIdentity(String domain) {
        return null;
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(getName()).toHashCode();
    }

    @Override
    public boolean equals(Object obj) {
        return this.getName().equals(((ExternalAlias) obj).getName());
    }

    public void setFiles(List<File> files) {
        m_files = files;
    }

    @Override
    public boolean isValidUser() {
        return true;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        return Collections.EMPTY_MAP;
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }

    @Override
    public boolean isEnabled() {
        return true;
    }
}
