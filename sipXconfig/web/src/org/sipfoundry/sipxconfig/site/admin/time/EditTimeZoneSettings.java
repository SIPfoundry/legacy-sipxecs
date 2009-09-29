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

import com.davekoelle.AlphanumComparator;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.time.TimeManager;
import org.sipfoundry.sipxconfig.admin.time.Timezone;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class EditTimeZoneSettings extends BaseComponent implements PageBeginRenderListener {
    @Parameter(required = true)
    public abstract TimeManager getTimeManager();

    public abstract String getTimezoneType();

    public abstract void setTimezoneType(String type);

    public abstract IPropertySelectionModel getTimezoneTypeModel();

    public abstract void setTimezoneTypeModel(IPropertySelectionModel model);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public void pageBeginRender(PageEvent event_) {
        // Init. the timezone dropdown menu.
        Timezone timezone = new Timezone();
        List<String> timezoneList = timezone.getAllTimezones();

        // Sort list alphanumerically.
        Collections.sort(timezoneList, new AlphanumComparator());
        StringPropertySelectionModel model = new StringPropertySelectionModel(timezoneList
                .toArray(new String[timezoneList.size()]));
        setTimezoneTypeModel(model);

        // Set the dropdown list to display the current time zone.
        // Note: getAllTimezones will have added the current timezone to
        // the dropdown list, if it wasn't in it.
        setTimezoneType(timezone.getTimezone());

        TimeZone.setDefault(TimeZone.getTimeZone(timezone.getTimezone()));
    }

    public void setSysTimezone() {
        getTimeManager().setSystemTimezone(getTimezoneType());
    }

}
