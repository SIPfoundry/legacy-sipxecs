/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import javax.servlet.http.HttpServletResponse;

import org.apache.commons.io.IOUtils;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.state.ApplicationStateManager;
import org.apache.tapestry.services.LinkFactory;
import org.apache.tapestry.util.ContentType;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.site.UserSession;

/**
 * Base class for services that need to serve files to end user
 */
public abstract class FileService implements IEngineService {

    private LinkFactory m_linkFactory;

    private FileDigestSource m_digestSource = new FileDigestSource();

    private WebResponse m_response;

    private ApplicationStateManager m_stateManager;

    public Integer getUserId() {
        if (m_stateManager.exists(UserSession.SESSION_NAME)) {
            UserSession userSession = (UserSession) m_stateManager.get(UserSession.SESSION_NAME);
            return userSession.getUserId();
        }
        return null;
    }

    public void setLinkFactory(LinkFactory linkFactory) {
        m_linkFactory = linkFactory;
    }

    public void setResponse(WebResponse response) {
        m_response = response;
    }


    public FileDigestSource getDigestSource() {
        return m_digestSource;
    }

    public void setDigestSource(FileDigestSource digestSource) {
        m_digestSource = digestSource;
    }

    public void setStateManager(ApplicationStateManager stateManager) {
        m_stateManager = stateManager;
    }

    protected void sendFile(File file, String expectedMd5Digest, ContentType contentType) throws IOException {
        Integer userId = getUserId();
        if (userId == null) {
            m_response.sendError(HttpServletResponse.SC_UNAUTHORIZED, file.getPath());
            return;
        }

        if (!file.canRead()) {
            m_response.sendError(HttpServletResponse.SC_NOT_FOUND, file.getPath());
            return;
        }

        String actualMd5Digest = m_digestSource.getDigestForResource(userId, file.getPath());
        if (!actualMd5Digest.equals(expectedMd5Digest)) {
            m_response.sendError(HttpServletResponse.SC_FORBIDDEN, file.getPath());
            return;
        }

        m_response.setHeader("Expires", "0");
        m_response.setHeader("Cache-Control", "must-revalidate, post-check=0, pre-check=0");
        m_response.setHeader("Pragma", "public");
        m_response.setHeader("Content-Disposition", "attachment; filename=\"" + file.getName()
                + "\"");

        OutputStream responseOutputStream = m_response.getOutputStream(contentType);
        InputStream stream = new FileInputStream(file);
        IOUtils.copy(stream, responseOutputStream);
    }

    protected Integer requireUserId() {
        Integer userId = getUserId();
        if (userId == null) {
            throw new RuntimeException("You have to be logged in to generate download links.");
        }
        return userId;
    }

    public LinkFactory getLinkFactory() {
        return m_linkFactory;
    }
}
