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
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.services.ServiceConstants;
import org.apache.tapestry.util.ContentType;

public class DownloadService extends FileService {
    public static final String SERVICE_NAME = "download";

    private static final String PARAM_PATH = "path";

    private static final String PARAM_CONTENT_TYPE = "contentType";

    private static final String PARAM_DIGEST = "digest";

    public String getName() {
        return SERVICE_NAME;
    }

    /**
     * The only parameter is the service parameters[dirName, fileName]
     */
    public void service(IRequestCycle cycle) throws IOException {
        File file = getFile(cycle);
        String expectedMd5Digest = cycle.getParameter(PARAM_DIGEST);
        ContentType contentType = getContentType(cycle);
        sendFile(file, expectedMd5Digest, contentType);
    }

    /**
     * Retrieves the file object from service parameters (filename, content type)
     *
     * @param cycle
     * @return nely created file object
     */
    private File getFile(IRequestCycle cycle) {
        String fileName = cycle.getParameter(PARAM_PATH);
        return new File(fileName);
    }

    /**
     * Retrieves the content type from service parameters
     *
     * @param cycle
     * @return nely created file object
     */
    private ContentType getContentType(IRequestCycle cycle) {
        String contentType = cycle.getParameter(PARAM_CONTENT_TYPE);
        return new ContentType(contentType);
    }

    public ILink getLink(boolean post, Object parameter) {
        Integer userId = requireUserId();
        DownloadLink.Info info = (DownloadLink.Info) parameter;

        Map parameters = new HashMap();

        parameters.put(ServiceConstants.SERVICE, getName());
        parameters.put(PARAM_PATH, info.getPath());
        parameters.put(PARAM_CONTENT_TYPE, info.getContentType());
        String digest = getDigestSource().getDigestForResource(userId, info.getPath());
        parameters.put(PARAM_DIGEST, digest);

        return getLinkFactory().constructLink(this, post, parameters, false);
    }

}
