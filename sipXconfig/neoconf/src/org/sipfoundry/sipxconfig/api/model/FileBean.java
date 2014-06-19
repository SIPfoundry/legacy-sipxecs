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
import java.nio.file.Files;
import java.nio.file.Path;

import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlType;

@XmlRootElement(name = "File")
@XmlType(propOrder = {
        "name", "type", "size", "path"
        })
public class FileBean {
    private String m_name;
    private String m_type;
    private long m_size;
    private String m_path;

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getType() {
        return m_type;
    }

    public void setType(String type) {
        m_type = type;
    }

    public long getSize() {
        return m_size;
    }

    public void setSize(long size) {
        m_size = size;
    }

    public String getPath() {
        return m_path;
    }

    public void setPath(String path) {
        m_path = path;
    }

    public static FileBean convertFile(File file) {
        if (!file.exists()) {
            return null;
        }
        FileBean bean = new FileBean();
        Path path = file.toPath();
        bean.setName(file.getName());
        bean.setPath(file.getAbsolutePath());
        try {
            bean.setSize(Files.size(path));
            bean.setType(Files.probeContentType(file.toPath()));
        } catch (Exception ex) {
            bean.setSize(-1);
            bean.setType("Unknown");
        }
        return bean;
    }

}
