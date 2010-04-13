/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting;

public class DelegatingSettingModel implements SettingModel {
    private final SettingModel m_model;
    private final SettingValueFilter m_filter;

    public DelegatingSettingModel(SettingModel model, SettingValueFilter filter) {
        m_model = model;
        m_filter = filter;
    }

    public SettingValue getDefaultSettingValue(Setting setting) {
        SettingValue sv = m_model.getDefaultSettingValue(setting);
        return m_filter.filter(sv);
    }

    public SettingValue getProfileName(Setting setting) {
        return m_model.getProfileName(setting);
    }

    public SettingValue getSettingValue(Setting setting) {
        SettingValue sv = m_model.getSettingValue(setting);
        return m_filter.filter(sv);
    }

    public void setSettingValue(Setting setting, String value) {
        m_model.setSettingValue(setting, value);
    }

    public static class InsertValueFilter extends AbstractSettingVisitor {
        private SettingValueFilter m_filter;

        public InsertValueFilter(SettingValueFilter filter) {
            m_filter = filter;
        }

        public void visitSetting(Setting setting) {
            // FIXME: downcast should not be necessary
            if (setting instanceof SettingImpl) {
                SettingImpl impl = (SettingImpl) setting;
                SettingModel origModel = impl.getModel();
                DelegatingSettingModel newModel = new DelegatingSettingModel(origModel, m_filter);
                impl.setModel(newModel);
            }
        }
    }
}
