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

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.file.Files;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;

import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.StreamingOutput;
import javax.ws.rs.core.Response.ResponseBuilder;
import javax.ws.rs.core.Response.Status;

import org.sipfoundry.sipxconfig.api.model.FileBean;
import org.sipfoundry.sipxconfig.api.model.SettingBean;
import org.sipfoundry.sipxconfig.api.model.SettingsList;
import org.sipfoundry.sipxconfig.api.model.ServerBean.JobList;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public final class ResponseUtils {
    public static final String CONTENT_LENGTH = "Content-Length";
    public static final String CONTENT_DISPOSITION = "Content-Disposition";

    private ResponseUtils() {
    }

    public static Response buildSettingResponse(BeanWithSettings bean, String path, Locale locale) {
        if (bean != null) {
            Setting settings = bean.getSettings();
            Setting setting = settings.getSetting(path);
            if (setting != null && !setting.isHidden()) {
                if (setting.isLeaf()) {
                    return Response.ok().entity(SettingBean.convertSetting(setting, locale)).build();
                } else {
                    return Response.ok().entity(SettingsList.convertSettingsList(setting, locale)).build();
                }
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    public static Response buildJobListResponse(List<Job> jobs, Location location, Locale locale) {
        if (location != null) {
            List<Job> serverJobs = new LinkedList<Job>();
            for (Job job : jobs) {
                Location jobLocation = job.getLocation();
                if (location != null && jobLocation.getId().equals(location.getId())) {
                    serverJobs.add(job);
                }
            }
            return Response.ok().entity(JobList.convertJobList(serverJobs, locale)).build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    public static Response buildDownloadFileResponse(File file) {
        FileBean bean = FileBean.convertFile(file);
        ResponseBuilder responseBuilder = Response.ok(file, bean.getType());
        responseBuilder.header(CONTENT_DISPOSITION, "attachment; filename=" + bean.getName());
        responseBuilder.header(CONTENT_LENGTH, bean.getSize());
        return responseBuilder.build();
    }

    public static Response buildStreamFileResponse(final File file) {
        StreamingOutput stream = new StreamingOutput() {
            @Override
            public void write(OutputStream outputStream) throws IOException, WebApplicationException {
                Files.copy(file.toPath(), outputStream);
            }
        };
        FileBean bean = FileBean.convertFile(file);
        ResponseBuilder responseBuilder = Response.ok(stream, bean.getType());
        responseBuilder.header(CONTENT_LENGTH, bean.getSize());
        return responseBuilder.build();
    }
}
