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

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class PhoneCloud extends BaseComponent {

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    // Allows pseudo device discovery by showing phone page
    // with filter set to show unassigned phone
    @InjectPage(value = ManagePhones.PAGE)
    public abstract ManagePhones getManagePhonesPage();

    @InjectPage(value = EditPhone.PAGE)
    public abstract EditPhone getEditPhonePage();

    public abstract List getPhones();

    public abstract void setPhones(List phones);

    public abstract int getCount();

    public abstract void setCount(int count);

    public abstract Phone getPhone();

    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (getPhones() == null) {
            PhoneContext phoneContext = getPhoneContext();
            // TODO: replace by loading lightweight user substitute
            List<Phone> phones = phoneContext.loadPhonesByPage(null, 0, 25, new String[] {
                "serialNumber"
            }, true);
            setPhones(phones);
            setCount(phoneContext.getPhonesCount());
        }
    }

    public IPage edit(Integer phoneId) {
        EditPhone page = getEditPhonePage();
        page.setPhoneId(phoneId);
        page.setReturnPage(getPage());
        return page;
    }

    // DirectLink listener to display phone page
    public IPage discover() {
        ManagePhones page = getManagePhonesPage();
        page.setDiscovery(true);
        return page;
    }

    public String getTitle() {
        return getMessages().format("title", getCount());
    }
}
