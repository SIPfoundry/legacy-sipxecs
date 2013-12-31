/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.sipfoundry.sipxconfig.hoteling.HotelingLocator;
import org.sipfoundry.sipxconfig.site.common.BeanNavigation;

/**
 * Top portion of pages that show tabs, help box, intro text, etc
 */
public abstract class PhoneNavigation extends BeanNavigation {
    @InjectObject(value = "spring:hotelingLocator")
    public abstract HotelingLocator getHotellingLocator();

    @InjectPage(value = PhoneSettings.PAGE)
    public abstract PhoneSettings getPhoneSettingsPage();

    @InjectPage(value = PhoneLines.PAGE)
    public abstract PhoneLines getPhoneLinesPage();

    @InjectPage(value = EditPhone.PAGE)
    public abstract EditPhone getEditPhonePage();

    public IPage editPhone(Integer phoneId) {
        EditPhone page = getEditPhonePage();
        page.setPhoneId(phoneId);
        return page;
    }

    public boolean isIdentificationTabActive() {
        return EditPhone.PAGE.equals(getPage().getPageName());
    }

    public boolean isLinesTabActive() {
        return PhoneLines.PAGE.equals(getPage().getPageName());
    }

    public IPage editLines(Integer phoneId) {
        PhoneLines page = getPhoneLinesPage();
        page.setPhoneId(phoneId);
        return page;
    }

    public IPage editSettings(Integer beanId, String section) {
        PhoneSettings page = getPhoneSettingsPage();
        page.setPhoneId(beanId);
        page.setParentSettingName(section);
        return page;
    }

    public String getGroupsToHide() {
        List<String> names = new LinkedList<String>();
        if (!isHotellingEnabled()) {
            names.add("prov");
        }
        names.add("group.version");
        return StringUtils.join(names, ",");
    }

    public boolean isHotellingEnabled() {
        return getHotellingLocator().isHotellingEnabled();
    }
}
