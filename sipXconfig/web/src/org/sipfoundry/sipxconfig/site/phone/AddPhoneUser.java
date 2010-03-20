/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.phone;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.user.UserTable;

public abstract class AddPhoneUser extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "phone/AddPhoneUser";

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    /** REQUIRED PROPERTY */
    public abstract Integer getPhoneId();

    public abstract void setPhoneId(Integer id);

    public abstract PhoneContext getPhoneContext();

    public abstract CoreContext getCoreContext();

    /**
     * made no attempts to abstract this to return to generic page because return page needs to
     * get state (phone id) returned This should be replaced with generic BreadCrumbs or
     * PageFlowGraph utility class when it's invented.
     */
    public abstract void setReturnToEditPhone(boolean rtn);

    public abstract boolean getReturnToEditPhone();

    public IPage select(IRequestCycle cycle) {
        PhoneContext context = getPhoneContext();

        UserTable table = (UserTable) getComponent("searchResults");
        SelectMap selections = table.getSelections();
        context.addUsersToPhone(getPhoneId(), selections.getAllSelected());

        PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
        page.setPhoneId(getPhoneId());
        return page;
    }

    public IPage cancel(IRequestCycle cycle) {
        IPage pageToActivate = null;
        if (getReturnToEditPhone()) {
            EditPhone page = (EditPhone) cycle.getPage(EditPhone.PAGE);
            page.setPhoneId(getPhoneId());
            pageToActivate = page;
        } else {
            PhoneLines page = (PhoneLines) cycle.getPage(PhoneLines.PAGE);
            page.setPhoneId(getPhoneId());
            pageToActivate = page;
        }
        return pageToActivate;
    }

    public void pageBeginRender(PageEvent event_) {
        Phone phone = getPhone();
        if (phone == null) {
            phone = getPhoneContext().loadPhone(getPhoneId());
            setPhone(phone);
        }
    }
}
