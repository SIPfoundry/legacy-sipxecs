/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.time;

import java.util.Collections;
import java.util.List;
import java.util.TimeZone;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.time.NtpManager;

import com.davekoelle.AlphanumComparator;

public abstract class EditTimeZoneSettings extends BaseComponent implements PageBeginRenderListener {
    @Parameter(required = true)
    public abstract NtpManager getTimeManager();

    public abstract String getTimezoneType();

    public abstract void setTimezoneType(String type);

    public abstract IPropertySelectionModel getTimezoneTypeModel();

    public abstract void setTimezoneTypeModel(IPropertySelectionModel model);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void pageBeginRender(PageEvent event_) {
        // Init. the timezone dropdown menu.
        List<String> timezoneList = getTimeManager().getAvailableTimezones();

        // Sort list alphanumerically.
        Collections.sort(timezoneList, new AlphanumComparator());
        StringPropertySelectionModel model = new StringPropertySelectionModel(timezoneList
                .toArray(new String[timezoneList.size()]));
        setTimezoneTypeModel(model);

        // Set the dropdown list to display the current time zone.
        // Note: getAllTimezones will have added the current timezone to
        // the dropdown list, if it wasn't in it.
        String tz = getTimeManager().getSystemTimezone();
        setTimezoneType(tz);

        if (tz != null) {
            TimeZone.setDefault(TimeZone.getTimeZone(tz));
        }

    }

    public void setSysTimezone() {
        getTimeManager().setSystemTimezone(getTimezoneType());
    }

}
