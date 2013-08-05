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
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.web.WebContext;
import org.sipfoundry.sipxconfig.device.HotellingManager;
import org.sipfoundry.sipxconfig.site.SpringBeanFactoryHolderImpl;
import org.sipfoundry.sipxconfig.site.common.BeanNavigation;
import org.springframework.beans.factory.ListableBeanFactory;

/**
 * Top portion of pages that show tabs, help box, intro text, etc
 */
public abstract class PhoneNavigation extends BeanNavigation implements PageBeginRenderListener {
    private HotellingManager m_hotellingManager;

    @InjectPage(value = PhoneSettings.PAGE)
    public abstract PhoneSettings getPhoneSettingsPage();

    @InjectPage(value = PhoneLines.PAGE)
    public abstract PhoneLines getPhoneLinesPage();

    @InjectPage(value = EditPhone.PAGE)
    public abstract EditPhone getEditPhonePage();

    @InjectObject(value = "service:tapestry.globals.WebContext")
    public abstract WebContext getWebContext();

    @Override
    public void pageBeginRender(PageEvent event) {
        ListableBeanFactory factory = SpringBeanFactoryHolderImpl.getWebApplicationContext(getWebContext());

        Map<String, HotellingManager> managers = factory.getBeansOfType(HotellingManager.class);
        if (!managers.isEmpty())
            for (String key : managers.keySet()) {
                m_hotellingManager = managers.get(key);
            }
    }

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
        if (m_hotellingManager == null) {
            return false;
        }
        return m_hotellingManager.isActive();
    }
}
