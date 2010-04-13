/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.enums.Enum;

/**
 * Capture static details about a bean. Examples: types of phones, types of uploads
 */
public abstract class BeanEnum extends Enum {

    private String m_beanId;

    private String m_label;

    private String m_enumId;

    public BeanEnum(String beanId, String label) {
        this(beanId, StringUtils.EMPTY, label);
    }

    public BeanEnum(String beanId, String enumId, String label) {
        super(beanId + enumId);
        m_beanId = beanId;
        m_enumId = enumId;
        m_label = label;
    }

    /**
     * Spring bean name, NOTE: You cannot reuse java class to multiple spring beans. One class/bean
     * but a spring bean can handle multiple models
     */
    public String getBeanId() {
        return m_beanId;
    }

    /**
     * User identifiable label for this model
     */
    public String getLabel() {
        return m_label;
    }

    protected String getEnumId() {
        return m_enumId;
    }
}
