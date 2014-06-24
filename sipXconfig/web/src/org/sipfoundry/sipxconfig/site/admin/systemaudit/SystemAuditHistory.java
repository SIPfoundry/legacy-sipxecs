/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 */

package org.sipfoundry.sipxconfig.site.admin.systemaudit;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Date;
import java.util.HashSet;
import java.util.Set;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.contrib.table.model.IBasicTableModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.EnumPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.cdr.CdrHistory;
import org.sipfoundry.sipxconfig.site.user.EditUser;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeAction;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeContext;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditFilter;

public abstract class SystemAuditHistory extends BaseComponent implements PageBeginRenderListener {

    public static final String PAGE = "admin/systemaudit/ManageSystemAudit";

    private static final Log LOG = LogFactory.getLog(SystemAuditHistory.class);
    private static final String SESSION = "session";

    @Persist(value = SESSION)
    public abstract Date getStartDate();
    public abstract void setStartDate(Date date);

    @Persist(value = SESSION)
    public abstract Date getEndDate();
    public abstract void setEndDate(Date date);

    @Persist(value = SESSION)
    public abstract ConfigChangeType getType();
    public abstract void setType(ConfigChangeType type);

    public IPropertySelectionModel getTypeModel() {
        EnumPropertySelectionModel typeModel = new EnumPropertySelectionModel() {
            @Override
            public String getLabel(int index) {
                return super.getOption(index).toString();
            }
        };
        typeModel.setEnumClass(ConfigChangeType.class);
        return new LocalizedOptionModelDecorator(typeModel, getMessages(), null);
    }

    @Persist(value = SESSION)
    public abstract ConfigChangeAction getAction();
    public abstract void setAction(ConfigChangeAction action);

    public IPropertySelectionModel getActionModel() {
        EnumPropertySelectionModel actionModel = new EnumPropertySelectionModel() {
            @Override
            public boolean isDisabled(int index) {
                if (getType() == null) {
                    return super.isDisabled(index);
                }
                ConfigChangeAction configChangeAction = ConfigChangeAction.getEnum(String.valueOf(index));
                return getType().isActionDisabled(configChangeAction);
            }

            @Override
            public String getLabel(int index) {
                return super.getOption(index).toString();
            }

            @Override
            public Object translateValue(String value) {
                if (getType() == null) {
                    return super.translateValue(value);
                } else {
                    int index = Integer.parseInt(value);
                    if (isDisabled(index)) {
                        return super.getOption(0);
                    } else {
                        return super.translateValue(value);
                    }
                }
            }
        };
        actionModel.setEnumClass(ConfigChangeAction.class);
        return new LocalizedOptionModelDecorator(actionModel, getMessages(), null);
    }

    @Persist(value = SESSION)
    public abstract String getGroupsString();
    public abstract void setGroupsString(String groups);

    @Persist(value = SESSION)
    public abstract String getUser();
    public abstract void setUser(String userName);

    @Persist(value = SESSION)
    public abstract String getDetails();
    public abstract void setDetails(String details);

    @InjectObject(value = "spring:configChangeContext")
    public abstract ConfigChangeContext getConfigChangeContext();

    @InjectObject(value = "spring:settingDao")
    public abstract SettingDao getSettingDao();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getGroupId();
    public abstract void setGroupId(Integer groupId);

    public IPage editUser(IRequestCycle cycle, Integer userId) {
        EditUser page = (EditUser) cycle.getPage(EditUser.PAGE);
        page.setUserId(userId);
        page.setReturnPage(PAGE);
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getEndDate() == null) {
            setEndDate(CdrHistory.getDefaultEndTime());
        }

        if (getStartDate() == null) {
            Date startTime = CdrHistory.getDefaultStartTime(getEndDate());
            setStartDate(startTime);
        }

        if (getStartDate().after(getEndDate())) {
            getValidator().record(new UserException("&log.invalidDates"), getMessages());
            return;
        }
    }

    public IBasicTableModel getTableModel() {
        return new ConfigChangeTableModel(getConfigChangeContext(),
                getGroupId(), getCurrentFilter());
    }

    private Set<Group> getGroups() {
        String groupsString = getGroupsString();
        Set<Group> groups = new HashSet<Group>();
        if (groupsString != null) {
            groups.addAll(getSettingDao().getGroupsByString(
                    User.GROUP_RESOURCE_ID, groupsString, false));
        }
        return groups;
    }

    @InjectObject(value = "service:tapestry.globals.WebResponse")
    public abstract WebResponse getResponse();

    public void export() {
        try {
            PrintWriter writer = TapestryUtils.getCsvExportWriter(
                    getResponse(), "system_audit.csv");
            getConfigChangeContext().dumpSystemAuditLogs(writer,
                    getCurrentFilter());
            writer.close();
        } catch (IOException e) {
            LOG.error("Error during System Audit export", e);
        }
    }

    private SystemAuditFilter getCurrentFilter() {
        SystemAuditFilter filter = new SystemAuditFilter(getStartDate(),
                getEndDate(), getType(), getAction(), getUser(), getDetails(),
                getGroups());
        return filter;
    }
}
