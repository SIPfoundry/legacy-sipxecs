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
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.LocalizationUtils;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.common.BeanNavigation;
import org.springframework.context.MessageSource;

/**
 * Control navigation of a sidebar in following setup :
 *
 *   +-----------------+------------------+
 *   +  page1          |                  |
 *   +  page2          |                  |
 *   +  page3          |                  |
 *   +  setting-group1 |    form          |
 *   +  setting-group2 |                  |
 *   +  setting-group3 |                  |
 *   +  ...            |                  |
 *   +-----------------+------------------+
 */
public abstract class SettingsNavigation extends BeanNavigation {
    @Parameter(required = true)
    public abstract IActionListener getEditSettingsListener();

    public Setting getSettings() {
        return getBean().getSettings();
    }

    public abstract void setCurrentSetting(Setting setting);

    public abstract Setting getCurrentSetting();

    public abstract MessageSource getMessageSource();

    public abstract void setMessageSource(MessageSource messageSource);

    @Parameter()
    public abstract void setActiveSetting(Setting setting);

    /**
     * Only required if settings parameter not set
     */
    @Parameter()
    public abstract Collection<Setting> getSource();

    @Parameter()
    public abstract String getGroupsToHide();

    /**
     * @return null when not navigating settings, e.g. page1, page2, ...
     */
    @Override
    public abstract Setting getActiveSetting();

    public String getCurrentSettingLabel() {
        Setting setting = getCurrentSetting();
        return LocalizationUtils.getSettingLabel(this, setting);
    }

    public boolean isSettingsTabActive() {
        Setting setting = getActiveSetting();
        if (setting == null) {
            return false;
        }

        return getCurrentSetting().getName().equals(setting.getName());
    }

    public Collection<Setting> getNavigationGroups() {
        Collection<Setting> s = getSource();
        if (s == null) {
            Setting settings = getSettings();
            if (settings != null) {
                s = getSettings().getValues();
            }
        }

        // creates a copy of s and work on it, in order to avoid ConcurrentModificationException
        if (s == null) {
            return s;
        }
        Collection<Setting> copyOfS = new ArrayList<Setting>();
        for (Setting setting : s) {
            copyOfS.add(setting);
        }

        if (getGroupsToHide() != null) {
            List<String> groupsToHide = Arrays.asList(getGroupsToHide().split(","));
            for (Setting setting : s) {
                if (groupsToHide.contains(setting.getName())) {
                    copyOfS.remove(setting);
                }
            }
        }
        return copyOfS;
    }

    @Override
    public void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        // set message source only once and save it into property so that we do not have to
        // compute it every time
        if (getMessageSource() == null) {
            Setting settings = getSettings();
            if (settings != null) {
                setMessageSource(settings.getMessageSource());
            }
        }
    }
}
