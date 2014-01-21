/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.backup;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.codehaus.jackson.map.ObjectMapper;
import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.StringRepresentation;
import org.restlet.resource.Variant;

public class UploadRestoreApi extends Resource {
    private String m_definitionId;
    private BackupManager m_backupManager;
    private RestoreApi m_restoreApi;
    private ObjectMapper m_jsonMapper = new ObjectMapper();

    @Override
    public void init(Context context, Request request, Response response) {
        // valid only for PUT
        m_definitionId = (String) getRequest().getAttributes().get("definitionId");
    }

    /**
     * GET
     *  Return list of files already uploaded
     */
    @Override
    public Representation represent(Variant variant) throws ResourceException {
        File d = m_backupManager.getRestoreStagingDirectory();
        d.list();
        StringWriter json = new StringWriter();
        String[] list;
        if (d.exists()) {
            list = d.list();
        } else {
            list = new String[0];
        }
        try {
            m_jsonMapper.writeValue(json, list);
        } catch (IOException e) {
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
        }
        return new StringRepresentation(json.toString());
    }

    /**
     * POST : Restore
     */
    @Override
    public void acceptRepresentation(Representation entity) throws ResourceException {
        m_restoreApi.acceptRepresentation(entity);
    }

    /**
     * PUT :
     *  With no definition id : reset uploads
     *  With definition id : Upload file
     */
    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        if (m_definitionId == null) {
            m_backupManager.getCleanRestoreStagingDirectory();
            throw new ResourceException(Status.SERVER_ERROR_INTERNAL, "Missing definition id");
        } else {
            FileOutputStream dstStream = null;
            try {
                File f = new File(m_backupManager.getRestoreStagingDirectory(), m_definitionId);
                dstStream = new FileOutputStream(f);
                IOUtils.copy(entity.getStream(), dstStream);
            } catch (IOException e) {
                throw new ResourceException(Status.SERVER_ERROR_INTERNAL, e.getMessage());
            } finally {
                IOUtils.closeQuietly(dstStream);
            }
        }
    }

    public boolean allowGet() {
        return true;
    }

    @Override
    public boolean allowPut() {
        return true;
    };

    @Override
    public boolean allowPost() {
        return true;
    }

    public void setBackupManager(BackupManager backupManager) {
        m_backupManager = backupManager;
    }

    public void setRestoreApi(RestoreApi restoreApi) {
        m_restoreApi = restoreApi;
    };
}
