/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.digester.Digester;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.xml.sax.SAXException;

public class ExternalAliases implements AliasProvider {
    private static final Log LOG = LogFactory.getLog(ExternalAliases.class);

    /** comma separated list of files dataset.alias.addins */
    private String m_aliasAddins;

    private String m_addinsDirectory = StringUtils.EMPTY;
    private CoreContext m_coreContext;

    private List getFiles() {
        if (StringUtils.isBlank(m_aliasAddins)) {
            return Collections.EMPTY_LIST;
        }
        File dir = new File(m_addinsDirectory);
        String[] names = StringUtils.split(m_aliasAddins, ", ");
        List files = new ArrayList(names.length);
        for (int i = 0; i < names.length; i++) {
            File file = new File(names[i]);
            if (file.exists()) {
                files.add(file);
                continue;
            }
            file = new File(dir, names[i]);
            if (file.exists()) {
                files.add(file);
                continue;
            }
            LOG.warn("File listed in dataset.alias.addins does not exist:" + file);
        }
        return files;
    }

    @SuppressWarnings("rawtypes")
    public Map<Replicable, Collection<AliasMapping>> getAliasMappings() {
        List files = getFiles();
        Map<Replicable, Collection<AliasMapping>> aliases = new HashMap<Replicable, Collection<AliasMapping>>();
        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        for (Iterator i = files.iterator(); i.hasNext();) {
            File file = (File) i.next();
            List<AliasMapping> parsedAliases = parseAliases(file);
            if (!CollectionUtils.isEmpty(parsedAliases)) {
                mappings.addAll(parsedAliases);
            }
        }
        if (!mappings.isEmpty()) {
            aliases.put(new ExternalAlias(), mappings);

            return aliases;
        }
        return Collections.EMPTY_MAP;
    }

    private List parseAliases(File file) {
        try {
            // Digester documentation advises against reusing digester objects
            Digester digester = ImdbXmlHelper.configureDigester(AliasMapping.class);
            return (List) digester.parse(file);
        } catch (IOException e) {
            LOG.warn("Errors when reading aliases file.", e);
        } catch (SAXException e) {
            LOG.warn("Errors when parsing aliases file.", e);
        }
        return null;
    }

    public void setAliasAddins(String aliasAddins) {
        m_aliasAddins = aliasAddins;
    }

    public void setAddinsDirectory(String addinsDirectory) {
        m_addinsDirectory = addinsDirectory;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
