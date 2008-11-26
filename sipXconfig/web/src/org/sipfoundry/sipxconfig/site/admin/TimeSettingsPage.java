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


import java.util.Collections;
import java.util.Date;
import java.util.List;

import com.davekoelle.AlphanumComparator;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.time.Timezone;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;



public abstract class TimeSettingsPage extends BasePage implements PageBeginRenderListener {
    @InjectObject(value = "spring:adminContext")
    public abstract AdminContext getAdminContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract Date getNewDate();

    public abstract void setNewDate(Date date);

    public abstract String getTimezoneType();

    public abstract void setTimezoneType(String type);

    public abstract IPropertySelectionModel getTimezoneTypeModel();

    public abstract void setTimezoneTypeModel(IPropertySelectionModel model);

    //
    // method to convert displayName to realName:
    // i.e. Replace Non-geographic with Etc.
    // This is done before call setSystemTimezone
    //
    private String convertDisplayNameToRealName(String displayName) {
        return (displayName.replaceAll("Non-geographic", "Etc"));
    }

    public void pageBeginRender(PageEvent event_) {
        setNewDate(new Date());

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
    }

    public void setDate() {
        getAdminContext().setSystemDate(getNewDate());
    }

    public void setSysTimezone() {
        String displayName = getTimezoneType();
        String realName = convertDisplayNameToRealName(displayName);
        getAdminContext().setSystemTimezone(realName);
    }

}
