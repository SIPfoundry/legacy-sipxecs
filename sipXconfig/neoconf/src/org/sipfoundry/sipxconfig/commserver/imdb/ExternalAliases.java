/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class ExternalAliases {
    private static final Log LOG = LogFactory.getLog(ExternalAliases.class);

    /** comma separated list of files dataset.alias.addins */
    private String m_aliasAddins;

    private String m_addinsDirectory = StringUtils.EMPTY;

    public List<File> getFiles() {
        if (StringUtils.isBlank(m_aliasAddins)) {
            return Collections.EMPTY_LIST;
        }
        File dir = new File(m_addinsDirectory);
        String[] names = StringUtils.split(m_aliasAddins, ", ");
        List<File> files = new ArrayList<File>(names.length);
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
    public Collection<AliasMapping> getAliasMappings() {
        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        ExternalAlias alias = new ExternalAlias();
        alias.setFiles(getFiles());
        return alias.getAliasMappings(null);
    }

    public void setAliasAddins(String aliasAddins) {
        m_aliasAddins = aliasAddins;
    }

    public void setAddinsDirectory(String addinsDirectory) {
        m_addinsDirectory = addinsDirectory;
    }

}
