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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.link.AbstractLinkComponent;
import org.apache.tapestry.link.StaticLink;

/**
 * Link that allows for downloading files from web server
 */
public abstract class DownloadLink extends AbstractLinkComponent {
    public abstract String getFileName();

    public abstract String getDirName();

    public abstract String getContentType();

    public abstract IEngineService getDownloadService();

    public ILink getLink(IRequestCycle cycle_) {
        IEngineService downloadService = getDownloadService();
        File file = getFile(getDirName(), getFileName());
        if (null == file) {
            // FIXME: need to make sure that this link is not rendered for non existing files...
            return new StaticLink("http://fixme/no/file/here/" + getFileName() + "/" + getDirName());
        }
        Info info = new Info(file.getAbsolutePath(), getContentType());
        return downloadService.getLink(false, info);
    }

    /**
     * Creates file object from directory name andbase file name.
     *
     * @param dirName directory name
     * @param fileName base file name
     * @return file object if it's possible to read from the file, null if not
     */
    private static File getFile(String dirName, String fileName) {
        if (null == dirName || null == fileName) {
            return null;
        }
        File file = new File(dirName, fileName);
        return file.canRead() ? file : null;
    }

    public static final class Info {
        private String m_path;
        private String m_contentType;

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
