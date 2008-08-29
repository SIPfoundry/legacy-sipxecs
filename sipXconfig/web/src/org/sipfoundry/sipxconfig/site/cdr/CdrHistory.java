/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import java.util.Calendar;
import java.util.Date;

import org.apache.commons.lang.time.DateUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.cdr.CdrSearch;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class CdrHistory extends BaseComponent implements PageBeginRenderListener {
    @Persist
    public abstract Date getStartTime();

    public abstract void setStartTime(Date startTime);

    @Persist
    public abstract Date getEndTime();

    public abstract void setEndTime(Date endTime);

    @Parameter
    public abstract User getUser();

    @Persist
    public abstract CdrSearch getCdrSearch();

    public abstract void setCdrSearch(CdrSearch cdrSearch);

    @Persist
    @InitialValue(value = "literal:active")
    public abstract String getTab();

    public void pageBeginRender(PageEvent event_) {
        if (getEndTime() == null) {
            setEndTime(getDefaultEndTime());
        }

        if (getStartTime() == null) {
            Date startTime = getDefaultStartTime(getEndTime());
            setStartTime(startTime);
        }

        if (getCdrSearch() == null) {
            setCdrSearch(new CdrSearch());
        }

        if (getStartTime().after(getEndTime())) {
            TapestryUtils.getValidator(getPage()).record(
                    new ValidatorException(getMessages().getMessage("message.invalidDates")));
            return;
        }
    }

    /**
     * By default set start at next midnight
     */
    public static Date getDefaultEndTime() {
        Calendar now = Calendar.getInstance();
        now.add(Calendar.DAY_OF_MONTH, 1);
        Calendar end = DateUtils.truncate(now, Calendar.DAY_OF_MONTH);
        return end.getTime();
    }

    /**
     *  start a day before end time
     */
    public static Date getDefaultStartTime(Date endTime) {
        Calendar then = Calendar.getInstance();
        then.setTime(endTime);
        then.add(Calendar.DAY_OF_MONTH, -1);
        return then.getTime();
    }
}
