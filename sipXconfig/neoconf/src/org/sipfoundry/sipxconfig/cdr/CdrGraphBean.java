/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import org.apache.commons.lang.StringUtils;

public class CdrGraphBean implements Comparable<CdrGraphBean> {
    private String m_key;

    private Integer m_count;

    public CdrGraphBean(String key, Integer count) {
        this.m_key = StringUtils.defaultString(key);
        this.m_count = count;
    }

    public String getKey() {
        return m_key;
    }

    public void setKey(String key) {
        m_key = key;
    }

    public Integer getCount() {
        return m_count;
    }

    public void setCount(Integer count) {
        m_count = count;
    }

    public int compareTo(CdrGraphBean obj) {
        if (obj == null) {
            return -1;
        }
        return getCount().compareTo(obj.getCount());
    }
}
