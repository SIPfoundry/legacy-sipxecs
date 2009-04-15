/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components.service;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.services.LinkFactory;
import org.apache.tapestry.util.ContentType;
import org.apache.tapestry.web.WebResponse;

public class ReportService implements IEngineService {
    public static final String SERVICE_NAME = "reportsService";

    private static final String PARAM_REPORT_PATH = "reportPath";

    private static final String PARAM_TYPE = "contentType";

    private LinkFactory m_linkFactory;

    private WebResponse m_response;

    public void service(IRequestCycle cycle) throws IOException {
        String path = cycle.getParameter(PARAM_REPORT_PATH);
        String contentType = cycle.getParameter(PARAM_TYPE);

        OutputStream responseOutputStream = m_response.getOutputStream(new ContentType(
                contentType));
        InputStream stream = new FileInputStream(path);
        IOUtils.copy(stream, responseOutputStream);
    }

    public ILink getLink(boolean post, Object obj) {
        Info info = (Info) obj;
        Map<String, String> params = new HashMap<String, String>();
        params.put(PARAM_REPORT_PATH, info.getPath());
        params.put(PARAM_TYPE, info.getContentType());
        return getLinkFactory().constructLink(this, post, params, false);
    }

    public String getName() {
        return SERVICE_NAME;
    }

    public void setLinkFactory(LinkFactory linkFactory) {
        m_linkFactory = linkFactory;
    }

    public LinkFactory getLinkFactory() {
        return m_linkFactory;
    }

    public void setResponse(WebResponse response) {
        m_response = response;
    }

    public static final class Info {
        private final String m_path;

        private final String m_contentType;

        public Info(String path, String contentType) {
            m_path = path;
            m_contentType = contentType;
        }

        public String getPath() {
            return m_path;
        }

        public String getContentType() {
            return m_contentType;
        }
    }
}
