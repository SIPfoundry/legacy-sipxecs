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

import org.sipfoundry.sipxconfig.common.BeanWithId;

public abstract class BeanWithSettings extends BeanWithId {
    private ModelFilesContext m_modelFilesContext;
    private Setting m_settings;
    private BeanWithSettingsModel m_model2;

    /**
     * While settings are getting decorated, this represents the settings that should be decorated
     */
    private Storage m_valueStorage;

    public BeanWithSettings() {
        initializeSettingModel();
    }

    protected void initializeSettingModel() {
        setSettingModel2(new BeanWithSettingsModel(this));
    }

    /**
     * Called the when someone needs to access settings for the first time.
     */
    protected void initialize() {
        // default implementation empty
    }

    protected void setSettingModel2(BeanWithSettingsModel model) {
        m_model2 = model;
    }

    protected BeanWithSettingsModel getSettingModel2() {
        return m_model2;
    }

    public void addDefaultSettingHandler(SettingValueHandler handler) {
        m_model2.addDefaultsHandler(handler);
    }

    public void addDefaultBeanSettingHandler(Object bean) {
        addDefaultSettingHandler(new BeanValueStorage(bean));
    }

    /**
     * @return decorated model - use this to modify phone settings
     */
    public Setting getSettings() {
        if (m_settings != null) {
            return m_settings;
        }
        setSettings(loadSettings());
        initialize();
        return m_settings;
    }

    protected abstract Setting loadSettings();

    public void setSettings(Setting settings) {
        m_settings = settings;
        m_model2.setSettings(m_settings);
    }

    public void setValueStorage(Storage valueStorage) {
        m_valueStorage = valueStorage;
    }

    public Storage getValueStorage() {
        return m_valueStorage;
    }

    protected synchronized Storage getInitializeValueStorage() {
        if (m_valueStorage == null) {
            setValueStorage(new ValueStorage());
        }

        return getValueStorage();
    }

    public String getSettingValue(String path) {
        return getSettings().getSetting(path).getValue();
    }

    public Object getSettingTypedValue(String path) {
        return getSettings().getSetting(path).getTypedValue();
    }

    public void setSettingValue(String path, String value) {
        Setting setting = getSettings().getSetting(path);
        setting.setValue(value);
    }

    public void setSettingTypedValue(String path, Object value) {
        Setting setting = getSettings().getSetting(path);
        setting.setTypedValue(value);
    }

    public void setModelFilesContext(ModelFilesContext modelFilesContext) {
        m_modelFilesContext = modelFilesContext;
    }

    public ModelFilesContext getModelFilesContext() {
        return m_modelFilesContext;
    }
}
