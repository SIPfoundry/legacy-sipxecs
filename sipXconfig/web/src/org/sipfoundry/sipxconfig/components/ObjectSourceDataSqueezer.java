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

import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

/**
 * Read objects by Id for Tapestry pages that need to read individual objects by their primary
 * key. For example table with select boxes.
 */
public class ObjectSourceDataSqueezer implements IPrimaryKeyConverter {
    private DataObjectSource m_source;

    private Class m_class;

    public ObjectSourceDataSqueezer(DataObjectSource source, Class klass) {
        setSource(source);
        setClass(klass);
    }

    public ObjectSourceDataSqueezer() {
        // empty bean constructor
    }

    public void setClass(Class klass) {
        if (!PrimaryKeySource.class.isAssignableFrom(klass)) {
            throw new IllegalArgumentException(
                    "Adapter only accepts classes that implement PrimaryKeySource");
        }
        m_class = klass;
    }

    public void setClassName(String className) {
        try {
            setClass(Class.forName(className));
        } catch (ClassNotFoundException e) {
            throw new RuntimeException(e);
        }
    }

    public void setSource(DataObjectSource source) {
        m_source = source;
    }

    public Object getPrimaryKey(Object objValue) {
        return objValue == null ? null : ((PrimaryKeySource) objValue).getPrimaryKey();
    }

    public Object getValue(Object objPrimaryKey) {
        return m_source.load(m_class, (Integer) objPrimaryKey);
    }

    public DataObjectSource getDataObjectSource() {
        return m_source;
    }
}
