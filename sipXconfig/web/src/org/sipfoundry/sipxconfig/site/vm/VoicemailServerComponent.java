/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.vm;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;
import org.sipfoundry.sipxconfig.site.common.GroupBooleanModel;

public abstract class VoicemailServerComponent extends BaseComponent {
    private static final String VOICEMAIL_SERVER_SETTING = "permission/voicemail-server";

    @Parameter(required = true)
    public abstract Setting getSettings();

    public abstract Setting getServer();

    public abstract void setServer(Setting serverSetting);

    public IPropertySelectionModel getModel() {
        ExtraOptionModelDecorator model = new ExtraOptionModelDecorator();
        model.setModel(new GroupBooleanModel(getSettings().getSetting(VOICEMAIL_SERVER_SETTING), this));
        model.setExtraLabel(getMessages().getMessage("option.selectDefault"));
        model.setExtraOption(null);
        return model;
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        SettingSetHelper helper = new SettingSetHelper(getSettings(), VOICEMAIL_SERVER_SETTING);
        if (!TapestryUtils.isRewinding(cycle, this)) {
            setServer(helper.getSelectedSetting());
        }
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            helper.setSelectedSetting(getServer());
        }
    }

    private static class SettingSetHelper {
        private final Setting m_rootSetting;
        private final String m_setPath;

        public SettingSetHelper(Setting rootSetting, String setPath) {
            m_rootSetting = rootSetting;
            m_setPath = setPath;
        }

        /**
         * Finds setting with "true" value. Returns null if all boolean settings are false.
         */
        public Setting getSelectedSetting() {
            Setting set = m_rootSetting.getSetting(m_setPath);
            for (Setting s : set.getValues()) {
                if (s.getType() instanceof BooleanSetting) {
                    if ((Boolean) s.getTypedValue()) {
                        return s;
                    }
                }
            }
            return null;
        }

        /**
         * Sets selected setting to true, all other settings to false
         */
        public void setSelectedSetting(Setting setting) {
            if (setting != null) {
                Setting set = m_rootSetting.getSetting(m_setPath);
                for (Setting s : set.getValues()) {
                    if (s.getType() instanceof BooleanSetting) {
                        s.setTypedValue(s == setting);
                    }
                }
            } else {
                Setting set = m_rootSetting.getSetting(m_setPath);
                for (Setting s : set.getValues()) {
                    if (s.getType() instanceof BooleanSetting) {
                        s.setValue(s.getDefaultValue());
                    }
                }
            }
        }
    }
}
