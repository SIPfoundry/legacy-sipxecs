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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditAutoAttendant extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "dialplan/EditAutoAttendant";

    @InjectObject(value = "spring:autoAttendantManager")
    public abstract AutoAttendantManager getAutoAttendantManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist(value = "client")
    @InitialValue(value = "literal:menu")
    public abstract String getTab();

    @Persist
    public abstract AutoAttendant getAttendant();

    public abstract void setAttendant(AutoAttendant attendant);

    public void reset() {
        getAttendant().resetToFactoryDefault();
    }

    public void commit() {
        IValidationDelegate validator = TapestryUtils.getValidator(this);
        if (!validator.getHasErrors()) {
            getAutoAttendantManager().storeAutoAttendant(getAttendant());
        }
    }

    public void pageBeginRender(PageEvent event_) {
        AutoAttendant aa = getAttendant();
        if (aa != null) {
            return;
        }
        aa = getAutoAttendantManager().newAutoAttendantWithDefaultGroup();
        aa.resetToFactoryDefault();
        setAttendant(aa);
    }

    public String getActionName(AttendantMenuAction action) {
        return getMessages().getMessage("menuItemAction." + action.getName());
    }
}
