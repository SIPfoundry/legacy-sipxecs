/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Date;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.login.LogFilter;
import org.sipfoundry.sipxconfig.login.LoginEvent;
import org.sipfoundry.sipxconfig.site.cdr.CdrHistory;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class LoginHistoryPage extends UserBasePage {

    public static final String PAGE = "LoginHistoryPage";

    private static final String CLIENT = "client";

    @Persist(value = CLIENT)
    public abstract Date getStartDate();

    public abstract void setStartDate(Date date);

    @Persist(value = CLIENT)
    public abstract Date getEndDate();

    public abstract void setEndDate(Date date);

    @Persist(value = CLIENT)
    public abstract String getQueryUser();

    public abstract void setQueryUser(String user);

    @Persist(value = CLIENT)
    public abstract String getQueryIp();

    public abstract void setQueryIp(String ip);

    @Persist(value = CLIENT)
    public abstract String getLogType();

    public abstract void setLogType(String type);

    public abstract String getLoginHistory();

    public abstract void setLoginHistory(String log);

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

        retrieveLogs();
    }

    public IPropertySelectionModel getLogTypeModel() {
        return new StringPropertySelectionModel(new String[] {
            getMessages().getMessage("log.type.all"),
            getMessages().getMessage(LoginEvent.SUCCESS),
            getMessages().getMessage(LoginEvent.FAILURE)
        });
    }

    public void filterLogs() {

    }

    private void retrieveLogs() {
        StringBuilder logContents = new StringBuilder();
        try {
            LogFilter filter = new LogFilter(getLogFilterType(), getStartDate(), getEndDate(),
                    getQueryUser(), getQueryIp());
            LoginEvent[] entries = getLoginContext().getUserLoginLog(filter);
            for (LoginEvent entry : entries) {
                logContents.append(entry.formatLogEntry());
                logContents.append(System.getProperty("line.separator"));
            }
        } catch (UserException ex) {
            logContents.append(getMessages().getMessage(ex.getMessage()));
        }

        setLoginHistory(logContents.toString());
    }

    private String getLogFilterType() {
        if (StringUtils.equals(getLogType(), getMessages().getMessage(LoginEvent.SUCCESS))) {
            return LoginEvent.SUCCESS;
        }
        if (StringUtils.equals(getLogType(), getMessages().getMessage(LoginEvent.FAILURE))) {
            return LoginEvent.FAILURE;
        }
        return null;
    }

}
