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
package org.sipfoundry.sipxconfig.admin.logging;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.core.io.FileSystemResource;

public class ReplicationBean implements ApplicationContextAware {
    private static final Log LOG = LogFactory.getLog(ReplicationBean.class);
    private static final String FAILED = "FAILED";
    private static final String MISSING_FLAG = " but flag is missing";
    private static final String FAILED_ON = "Replication failed on: ";
    private static final String STARTED_ON = "Replication started on: ";

    private Map<String, FileSystemResource> m_flags = new HashMap<String, FileSystemResource>();
    private Map<String, Boolean> m_failed = new HashMap<String, Boolean>();
    private String m_path;

    private ApplicationContext m_applicationContext;

    private FileSystemResource createResource(String fqdn) {
        FileSystemResource resource = new FileSystemResource(m_path);
        FileSystemResource fileResource = (FileSystemResource) resource.createRelative(fqdn);
        m_flags.put(fqdn, fileResource);
        return fileResource;
    }

    public boolean isFailed(String fqdn) {
        Boolean failed = m_failed.get(fqdn);
        if (failed != null && failed) {
            //already failed
            return true;
        }
        //verify if failed in a previous sipxecs run
        FileSystemResource resource = m_flags.get(fqdn);
        if (resource == null) {
            resource = createResource(fqdn);
        }
        try {
            String content = IOUtils.toString(resource.getInputStream());
            return StringUtils.equals(content, FAILED);
        } catch (Exception ex) {
            return false;
        }
    }

    public void createFlagFailed(String fqdn) {
        Boolean failed = m_failed.get(fqdn);
        if (failed != null && failed) {
            //flag already created
            return;
        }
        FileSystemResource resource = m_flags.get(fqdn);
        if (resource == null) {
            resource = createResource(fqdn);
        }
        try {
            IOUtils.write(FAILED, new FileOutputStream(resource.getFile()));
            m_failed.put(fqdn, true);
            //publish failed replication event
            FailedReplicationEvent event = new FailedReplicationEvent(fqdn);
            event.setFqdn(fqdn);
            m_applicationContext.publishEvent(event);
        } catch (FileNotFoundException ex) {
            LOG.error(FAILED_ON + fqdn + MISSING_FLAG);
        } catch (IOException ex) {
            LOG.error(FAILED_ON + fqdn + " but cannot create flag");
        }
    }

    public void removeFlag(String fqdn) {
        m_failed.remove(fqdn);
        FileSystemResource resource = m_flags.get(fqdn);
        if (resource == null) {
            resource = createResource(fqdn);
        }
        try {
            IOUtils.write("", new FileOutputStream(resource.getFile()));
        } catch (FileNotFoundException ex) {
            LOG.error(STARTED_ON + fqdn + MISSING_FLAG);
        } catch (IOException ex) {
            LOG.error(STARTED_ON + fqdn + " but cannot remove old flag");
        }
    }

    @Required
    public void setPath(String path) {
        m_path = path;
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }
}
