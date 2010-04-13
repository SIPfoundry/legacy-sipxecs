/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting.type;

public abstract class AbstractSettingType implements SettingType {
    private String m_id;

    public AbstractSettingType() {
        super();
    }

    public void setId(String id) {
        m_id = id;
    }

    protected String getId() {
        return m_id;
    }

    @Override
    public SettingType clone() {
        try {
            return (SettingType) super.clone();
        } catch (CloneNotSupportedException e) {
            throw new RuntimeException(e);
        }
    }
}
