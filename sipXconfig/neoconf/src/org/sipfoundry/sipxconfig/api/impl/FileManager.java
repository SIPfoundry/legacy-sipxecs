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
import java.io.InputStream;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.activation.DataHandler;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.cxf.jaxrs.ext.multipart.Attachment;
import org.sipfoundry.sipxconfig.api.model.FileList;

public class FileManager {
    private String m_path;
    private String[] m_extensions;

    public FileList getFileList() {
        Collection<File> files = FileUtils.listFiles(new File(m_path), m_extensions, true);
        return FileList.convertFileList(files);
    }

    public void deleteFile(String fileName) throws IOException {
        Files.deleteIfExists(new File(m_path, fileName).toPath());
    }

    public File getFile(String fileName) {
        return new File(m_path, fileName);
    }

    public boolean checkFile(String filename) {
        return new File(m_path, filename).exists();
    }

    public List<String> uploadFiles(List<Attachment> attachments) {
        List<String> failures = new ArrayList<String>();
        for (Attachment attachment : attachments) {
            String name = getFileNameFromContentDisposition(attachment
                    .getHeader(ResponseUtils.CONTENT_DISPOSITION));
            if (uploadFile(attachment.getDataHandler(), name)) {
                failures.add(name);
            }
        }
        return failures;
    }

    private String getFileNameFromContentDisposition(String header) {
        Pattern regex = Pattern.compile("(?<=filename=\").*?(?=\")");
        Matcher regexMatcher = regex.matcher(header);
        if (regexMatcher.find()) {
            return regexMatcher.group();
        }
        return null;
    }

    private boolean uploadFile(DataHandler handler, String fileName) {
        InputStream is = null;
        try {
            is = handler.getInputStream();
            File file = new File(m_path, fileName);
            Files.copy(is, file.toPath());
        } catch (IOException ex) {
            return false;
        } finally {
            IOUtils.closeQuietly(is);
        }
        return true;
    }

    public void setAudioPath(String path) {
        m_path = path;
    }

    public void setExtensions(List<String> extensions) {
        m_extensions = extensions.toArray(new String[extensions.size()]);
    }

}
