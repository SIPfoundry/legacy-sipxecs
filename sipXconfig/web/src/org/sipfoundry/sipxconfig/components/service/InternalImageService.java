/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components.service;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.asset.AssetService;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.services.LinkFactory;
import org.apache.tapestry.util.ContentType;
import org.apache.tapestry.web.WebResponse;

public class InternalImageService extends AssetService {
    public static final String SERVICE_NAME = "internalImageService";

    private static final String PARAM_PATH = "path";

    private LinkFactory m_linkFactory;

    private WebResponse m_response;

    public void service(IRequestCycle cycle) throws IOException {
        String path = cycle.getParameter(PARAM_PATH);
        File file = new File(path);
        String extension = file.getName().substring(file.getName().lastIndexOf(".") + 1,
                file.getName().length());
        OutputStream responseOutputStream = m_response.getOutputStream(new ContentType("image/"
                + extension));
        InputStream stream = new FileInputStream(path);
        IOUtils.copy(stream, responseOutputStream);
    }

    public ILink getLink(boolean post, Object info) {
        Map<String, String> params = new HashMap<String, String>();
        params.put(PARAM_PATH, ((Info) info).getPath());
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
        private String m_path;

        public Info(String path) {
            m_path = path;
        }

        public String getPath() {
            return m_path;
        }
    }
}
