/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.setting.GroupSettings;

public abstract class ManageAttendants extends SipxBasePage {

    public static final String PAGE = "dialplan/ManageAttendants";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @InjectPage(EditAutoAttendant.PAGE)
    public abstract EditAutoAttendant getEditPage();

    @InjectPage(EditAttendantDefaults.PAGE)
    public abstract GroupSettings getGroupSettingsPage();

    @InjectObject("spring:autoAttendantManager")
    public abstract AutoAttendantManager getAutoAttendantManager();

    @Persist
    @InitialValue(value = "literal:manage")
    public abstract String getTab();

    public abstract AutoAttendant getCurrentRow();

    public void deleteSelected() {
        Collection selectedRows = getSelections().getAllSelected();
        if (selectedRows != null) {
            AutoAttendantManager manager = getAutoAttendantManager();
            manager.deleteAutoAttendantsByIds(selectedRows);
        }
    }

    public IPage edit(Integer id) {
        EditAutoAttendant page = getEditPage();
        AutoAttendant attendant = getAutoAttendantManager().getAutoAttendant(id);
        page.setAttendant(attendant);
        page.setReturnPage(this);
        return page;
    }

    public IPage add() {
        EditAutoAttendant page = getEditPage();
        page.setAttendant(null);
        page.setReturnPage(this);
        return page;
    }

    public IPage defaultGroup() {
        GroupSettings page = getGroupSettingsPage();
        Group defaultGroup = getAutoAttendantManager().getDefaultAutoAttendantGroup();
        AutoAttendant aa = getAutoAttendantManager().newAutoAttendantWithDefaultGroup();
        page.editGroup(defaultGroup.getId(), aa, PAGE);
        return page;
    }
}
