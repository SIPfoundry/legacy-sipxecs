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

import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.phone.ManagePhones;

/**
 * Create a new line
 */
public abstract class NewLine extends SipxBasePage {

    public static final String PAGE = "NewLine";

    public abstract void setPhone(Phone phone);

    public abstract Phone getPhone();

    public abstract void setLine(Line line);

    public abstract Line getLine();

    public abstract int getPhoneId();

    public abstract void setPhoneId(int id);

    public abstract void setReturnPage(String returnPage);

    public abstract String getReturnPage();

    public abstract PhoneContext getPhoneContext();

    public String finish() {
        saveLine();
        if (isPhoneWizard()) {
            savePhone();
        }
        return getReturnPage();
    }

    public boolean isPhoneWizard() {
        return getPhone() != null;
    }

    void saveLine() {
        getPhoneContext().storeLine(getLine());
    }

    void savePhone() {
        // TODO: may not have line interface in 2.8
        throw new RuntimeException("not implemented");
    }

    public String cancel() {
        return ManagePhones.PAGE;
    }
}
