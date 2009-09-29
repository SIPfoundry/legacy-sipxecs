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

import java.util.ArrayList;
import java.util.List;

public class BeanWithSettingsModel implements SettingModel {
    private List<SettingValueHandler> m_defaultHandlers = new ArrayList<SettingValueHandler>();
    private SettingValueHandler m_defaultsHandler = new MulticastSettingValueHandler(m_defaultHandlers);
    private ProfileNameHandler m_defaultProfileNameHandler;
    private BeanWithSettings m_bean;

    BeanWithSettingsModel(BeanWithSettings bean) {
        m_bean = bean;
    }

    public void addDefaultsHandler(SettingValueHandler handler) {
        m_defaultHandlers.add(handler);
    }

    public void setDefaultProfileNameHandler(ProfileNameHandler handler) {
        m_defaultProfileNameHandler = handler;
    }

    public void setSettings(Setting settings) {
        if (settings != null) {
            settings.acceptVisitor(new SetModelReference(this));
        }
    }

    protected BeanWithSettings getBeanWithSettings() {
        return m_bean;
    }

    protected SettingValueHandler getDefaultsHandler() {
        return m_defaultsHandler;
    }

    public SettingValue getSettingValue(Setting setting) {
        SettingValue value = null;
        Storage vs = getBeanWithSettings().getValueStorage();
        if (vs != null) {
            value = vs.getSettingValue(setting);
        }

        if (value == null) {
            value = getDefault(setting);
        }

        return value;
    }

    public SettingValue getDefaultSettingValue(Setting setting) {
        // just do not consult the bean, that is the default
        return getDefault(setting);
    }

    protected SettingValue getDefault(Setting setting) {
        SettingValue value = getDefaultsHandler().getSettingValue(setting);
        return value;
    }

    /**
     * not pretty, but need to populate setting model into every setting instance
     */
    static class SetModelReference extends AbstractSettingVisitor {
        private SettingModel m_model;

        public SetModelReference(SettingModel model) {
            m_model = model;
        }

        public void visitSetting(Setting setting) {
            // FIXME: downcast should not be necessary
            if (setting instanceof SettingImpl) {
                SettingImpl impl = (SettingImpl) setting;
                impl.setModel(m_model);
            }
        }
    }

    public void setSettingValue(Setting setting, String sValue) {
        SettingValue defaultValue = new SettingValueImpl(setting.getDefaultValue());
        SettingValue value = new SettingValueImpl(sValue);
        Storage vs = m_bean.getInitializeValueStorage();
        vs.setSettingValue(setting, value, defaultValue);
    }

    public SettingValue getProfileName(Setting setting) {
        if (m_defaultProfileNameHandler != null) {
            return m_defaultProfileNameHandler.getProfileName(setting);
        }
        return null;
    }
}
