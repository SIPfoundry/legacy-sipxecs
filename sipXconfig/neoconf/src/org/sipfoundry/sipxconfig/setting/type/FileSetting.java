/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.setting.type;

import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import static java.io.File.separator;

import org.apache.commons.lang.StringUtils;

/**
 * Special type of setting used for upload-able file.
 */
public class FileSetting extends AbstractSettingType {
    private boolean m_required;
    private boolean m_variable;
    private final List<String> m_zipExcludes = new LinkedList();

    /** Directory in which downloaded files are stored */
    private String m_directory;

    /** Mime content type */
    private String m_contentType = "audio/x-wav";

    public boolean isRequired() {
        return m_required;
    }

    public void setRequired(boolean required) {
        m_required = required;
    }

    public boolean isVariable() {
        return m_variable;
    }

    public void setVariable(boolean variable) {
        m_variable = variable;
    }

    public String getDirectory() {
        return m_directory;
    }

    public void setDirectory(String directory) {
        m_directory = directory;
    }

    public void setContentType(String contentType) {
        m_contentType = contentType;
    }

    public String getContentType() {
        return m_contentType;
    }

    public String getName() {
        return "file";
    }

    public void addZipExclude(String filename) {
        if (StringUtils.isBlank(filename)) {
            return;
        }
        // trailing separator always
        String excludedName = filename + separator;
        // remove duplicated separators using greedy matcher
        excludedName = excludedName.replaceAll(separator + "++", separator);
        // only add if not already there...
        if (!m_zipExcludes.contains(excludedName)) {
            m_zipExcludes.add(excludedName);
        }
    }

    public boolean isExcluded(String entry) {
        // Ensure the entry has a trailing separator. (For both files and directories.)
        String candidate = entry;
        if (!candidate.endsWith(separator)) {
            candidate = candidate + separator;
        }

        // Search for a match. (The trailing separator prevents "zip/" from matching "zip-test/".)
        for (String exclude : getZipExcludes()) {
            if (candidate.startsWith(exclude)) {
                return true;
            }
        }
        return false;
    }

    public List<String> getZipExcludes() {
        return Collections.unmodifiableList(m_zipExcludes);
    }

    public Object convertToTypedValue(Object value) {
        return value;
    }

    public String convertToStringValue(Object value) {
        if (value == null) {
            return null;
        }
        return value.toString();
    }

    public String getLabel(Object value) {
        return convertToStringValue(value);
    }
}
