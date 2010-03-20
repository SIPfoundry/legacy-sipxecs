/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.line;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.phone.ManagePhones;


public abstract class EditLine extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "line/EditLine";

    public abstract Line getLine();

    public abstract void setLine(Line line);

    /** REQUIRED PROPERTY */
    public abstract Integer getLineId();

    public abstract void setLineId(Integer id);

    public abstract PhoneContext getPhoneContext();

    public String ok() {
        apply();
        return ManagePhones.PAGE;
    }

    public void apply() {
        PhoneContext dao = getPhoneContext();
        dao.storeLine(getLine());
        dao.flush();
    }

    public String cancel() {
        return ManagePhones.PAGE;
    }

    public void pageBeginRender(PageEvent event_) {
        PhoneContext context = getPhoneContext();
        setLine(context.loadLine(getLineId()));
    }
}
