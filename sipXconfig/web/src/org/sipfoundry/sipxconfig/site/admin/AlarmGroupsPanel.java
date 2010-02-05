/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmGroup;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class AlarmGroupsPanel extends BaseComponent implements PageBeginRenderListener {
    @InjectObject("spring:alarmServerManager")
    public abstract AlarmServerManager getAlarmServerManager();

    @InjectPage(EditAlarmGroupPage.PAGE)
    public abstract EditAlarmGroupPage getEditAlarmGroupPage();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    public abstract void setGroups(List<AlarmGroup> groups);

    public abstract List<AlarmGroup> getGroups();

    public abstract void setCurrentRow(AlarmGroup group);

    public abstract Collection getSelectedRows();

    public IPage addAlarmGroup() {
        EditAlarmGroupPage page = getEditAlarmGroupPage();
        page.addAlarmGroup(getPage());
        return page;
    }

    public IPage editAlarmGroup(Integer groupId) {
        EditAlarmGroupPage page = getEditAlarmGroupPage();
        page.editAlarmGroup(groupId, getPage());
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        // load alarm groups
        List<AlarmGroup> groups = getAlarmServerManager().getAlarmGroups();
        setGroups(groups);
    }

    public void delete() {
        Collection<Integer> allSelected = new ArrayList<Integer>(getAllSelected());
        if (allSelected.isEmpty()) {
            return;
        }

        boolean printErrorMessage = getAlarmServerManager().removeAlarmGroups(allSelected,
                getAlarmServerManager().getAlarmTypes());
        if (printErrorMessage) {
            IValidationDelegate validator = TapestryUtils.getValidator(getPage());
            validator.record(new ValidatorException(getMessages().getMessage("msg.err.defalutAlarmGroupDeletion")));
        }
    }

    public Collection getAllSelected() {
        return getSelections().getAllSelected();
    }
}
