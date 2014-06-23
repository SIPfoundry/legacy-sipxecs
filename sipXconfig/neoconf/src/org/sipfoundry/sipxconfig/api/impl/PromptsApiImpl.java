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
package org.sipfoundry.sipxconfig.api.impl;

import java.io.IOException;
import java.util.List;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.Status;

import org.apache.commons.lang.StringUtils;
import org.apache.cxf.jaxrs.ext.multipart.Attachment;
import org.sipfoundry.sipxconfig.api.PromptsApi;

public class PromptsApiImpl extends FileManager implements PromptsApi {
    @Override
    public Response getPrompts() {
        return Response.ok().entity(getFileList()).build();
    }

    @Override
    public Response uploadPrompts(List<Attachment> attachments, HttpServletRequest request) {
        List<String> failures = uploadFiles(attachments);
        if (failures.size() > 0) {
            return Response.serverError().entity(StringUtils.join(failures, ",")).build();
        }
        return Response.ok().build();
    }

    @Override
    public Response downloadPrompt(String promptName) {
        Response response = checkPrompt(promptName);
        if (response != null) {
            return response;
        }
        return ResponseUtils.buildDownloadFileResponse(getFile(promptName));
    }

    @Override
    public Response removePrompt(String promptName) {
        Response response = checkPrompt(promptName);
        if (response != null) {
            return response;
        }
        try {
            deleteFile(promptName);
        } catch (IOException exception) {
            return Response.serverError().entity(exception.getMessage()).build();
        }
        return Response.ok().build();
    }

    @Override
    public Response streamPrompt(String promptName) {
        Response response = checkPrompt(promptName);
        if (response != null) {
            return response;
        }
        return ResponseUtils.buildStreamFileResponse(getFile(promptName));
    }

    protected Response checkPrompt(String promptName) {
        if (promptName != null) {
            boolean fileExists = checkFile(promptName);
            if (!fileExists) {
                return Response.status(Status.NOT_FOUND).entity("Prompt not found").build();
            }
        }
        return null;
    }

}
