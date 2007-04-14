/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingArray;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.sipfoundry.sipxconfig.setting.SettingVisitor;
import org.springframework.context.MessageSource;

/**
 * Fieldset to display a collection of settings
 */
@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class SettingsFieldset extends BaseComponent {
    @Parameter(required = true)
    public abstract Setting getSettings();

    public abstract void setSettings(Setting setting);

    @Parameter(defaultValue = "true")
    public abstract boolean getRenderGroupTitle();

    public abstract void setRenderGroupTitle(boolean render);

    @Parameter(defaultValue = "true")
    public abstract void setEnabled(boolean enabled);

    @Parameter(defaultValue = "true")
    public abstract void setRequiredEnabled(boolean enabled);

    public abstract Collection<Setting> getFlattenedSettings();

    public abstract void setFlattenedSettings(Collection<Setting> flat);

    @Persist(value = "session")
    public abstract boolean getShowAdvanced();

    public abstract void setShowAdvanced(boolean showAdvanced);

    public abstract Setting getCurrentSetting();

    public abstract void setCurrentSetting(Setting setting);

    public abstract MessageSource getMessageSource();

    public abstract void setMessageSource(MessageSource setting);

    public abstract void setRenderAdvancedToggle(boolean advanced);

    public IComponent getCurrentBlock() {
        String blockName = "settingBlock";
        Setting currentSetting = getCurrentSetting();
        if (currentSetting instanceof SettingSet) {
            blockName = "groupBlock";
        } else if (currentSetting instanceof SettingArray) {
            blockName = "arrayBlock";
        }
        return getComponent(blockName);
    }

    /**
     * Render group if it's not advanced (hidden) or if show advanced is set and group title
     * rendering is allowed
     * 
     * @param setting
     * @return true if setting should be rendered
     */
    public boolean getRenderGroup() {
        Setting setting = getCurrentSetting();
        if (!getRenderGroupTitle()) {
            // group title rendering not allowed
            return false;
        }
        return showSetting(setting);
    }

    /**
     * Render setting if it's not advanced (hidden) or if show advanced is set
     * 
     * @param setting
     * @return true if setting should be rendered
     */
    public boolean getRenderSetting() {
        Setting setting = getCurrentSetting();
        return showSetting(setting);
    }

    boolean showSetting(Setting setting) {
        if (getShowAdvanced()) {
            return true;
        }
        return !SettingUtil.isAdvancedIncludingParents(getSettings(), setting);
    }

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        // set message source only once and save it into property so that we do not have to
        // compute it every time
        if (getMessageSource() == null) {
            setMessageSource(getSettings().getMessageSource());
        }
        // compute flattened settings
        if (getFlattenedSettings() == null) {
            SettingsIron iron = new SettingsIron();
            getSettings().acceptVisitor(iron);
            setFlattenedSettings(iron.getFlat());
            setRenderAdvancedToggle(iron.isAdvanced());
        }
    }

    public String getDescription() {
        Setting setting = getCurrentSetting();
        return TapestryUtils.getSettingDescription(this, setting);
    }

    public String getLabel() {
        Setting setting = getCurrentSetting();
        return TapestryUtils.getSettingLabel(this, setting);
    }

    /**
     * Strips hidden settings, checks for advanced flag. Please note that it's not a good idea to
     * strip advanced settings here. We need to render advanced settings as hidden components on
     * the page if user modifies them and then switches between 'advanced' and 'normal' views.
     */
    static class SettingsIron implements SettingVisitor {
        private Collection<Setting> m_flat = new ArrayList<Setting>();
        private boolean m_isAdvanced;

        public Collection<Setting> getFlat() {
            return m_flat;
        }

        public boolean isAdvanced() {
            return m_isAdvanced;
        }

        public void visitSetting(Setting setting) {
            if (setting.isHidden()) {
                return;
            }
            m_isAdvanced = m_isAdvanced || setting.isAdvanced();
            m_flat.add(setting);
        }

        public boolean visitSettingArray(SettingArray array) {
            visitSetting(array);
            // do not flatten array members
            return false;
        }

        public boolean visitSettingGroup(SettingSet group) {
            if (group.isHidden()) {
                return false;
            }
            visitSetting(group);
            return true;
        }
    }
}
