/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;
import java.util.Collections;

/**
 * Abstract setting with model (as in model/view/controller model)
 */
public class SettingImpl extends AbstractSetting {
    private SettingModel m_model;

    /**
     * bean access only, must set name before valid object
     */
    public SettingImpl() {
    }

    public SettingImpl(String name) {
        setName(name);
    }

    @Override
    public String getProfileName() {
        String profileName = super.getProfileName();
        if (m_model == null) {
            return profileName;
        }
        SettingValue modelProfileName = m_model.getProfileName(this);
        if (modelProfileName == null) {
            return profileName;
        }
        return modelProfileName.getValue();
    }

    @Override
    public String getValue() {
        if (m_model != null) {
            SettingValue value = m_model.getSettingValue(this);
            if (value != null) {
                return value.getValue();
            }
        }
        return super.getValue();
    }

    @Override
    public void setValue(String value) {
        if (m_model != null) {
            m_model.setSettingValue(this, value);
        } else {
            super.setValue(value);
        }
    }

    public String getDefaultValue() {
        if (m_model != null) {
            SettingValue value = m_model.getDefaultSettingValue(this);
            if (value != null) {
                return value.getValue();
            }
        }
        return super.getValue();
    }

    public Collection<Setting> getValues() {
        return Collections.emptyList();
    }

    public void setModel(SettingModel model) {
        m_model = model;
    }

    public SettingModel getModel() {
        return m_model;
    }

    public Setting addSetting(Setting setting_) {
        throw new UnsupportedOperationException("Cannot put settings into another setting, only groups");
    }

    @Override
    protected Setting findChild(String name_) {
        throw new UnsupportedOperationException("Cannot get settings from another setting, only groups");
    }
}
