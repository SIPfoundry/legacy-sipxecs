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
package org.sipfoundry.sipxconfig.api.model;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

@XmlRootElement(name = "Files")
public class FileList {

    private List<FileBean> m_files;

    public void setFiles(List<FileBean> files) {
        m_files = files;
    }

    @XmlElement(name = "File")
    public List<FileBean> getFiles() {
        if (m_files == null) {
            m_files = new ArrayList<FileBean>();
        }
        return m_files;
    }

    public static FileList convertFileList(Collection<File> filesList) {
        List<FileBean> files = new ArrayList<FileBean>();
        for (File file : filesList) {
            files.add(FileBean.convertFile(file));
        }
        FileList list = new FileList();
        list.setFiles(files);
        return list;
    }
}
