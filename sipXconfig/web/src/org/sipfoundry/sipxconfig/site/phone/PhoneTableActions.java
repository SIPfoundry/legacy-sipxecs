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

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.RestartManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class PhoneTableActions extends BaseComponent {
    @Parameter(required = true)
    public abstract SelectMap getSelections();

    /**
     * If set to true we will only present actions that makes sense on the user page
     */
    @Parameter(required = false, defaultValue = "ognl:false")
    public abstract boolean getUserOnly();

    @Parameter(required = false, defaultValue = "ognl:null")
    public abstract IPropertySelectionModel getActionModel();

    @Parameter(required = true)
    public abstract Collection<Integer> getGenerateProfileIds();

    public abstract void setGenerateProfileIds(Collection<Integer> ids);

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:phoneRestartManager")
    public abstract RestartManager getRestartManager();

    public void generateProfiles() {
        Collection ids = getSelections().getAllSelected();
        if (!ids.isEmpty()) {
            setGenerateProfileIds(ids);
        }
    }

    public void generateAllProfiles() {
        Collection ids = getPhoneContext().getAllPhoneIds();
        if (!ids.isEmpty()) {
            setGenerateProfileIds(ids);
        }
    }

    public void restart() {
        Collection phoneIds = getSelections().getAllSelected();
        getRestartManager().restart(phoneIds, null);
        String msg = getMessages().format("msg.success.restart", Integer.toString(phoneIds.size()));
        TapestryUtils.recordSuccess(this, msg);
    }

    public void deletePhone() {
        PhoneContext context = getPhoneContext();

        Collection ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }

        for (Iterator i = ids.iterator(); i.hasNext();) {
            Integer phoneId = (Integer) i.next();
            Phone phone = context.loadPhone(phoneId);
            context.deletePhone(phone);
        }

        String msg = getMessages().format("msg.success.delete", Integer.toString(ids.size()));
        TapestryUtils.recordSuccess(this, msg);
    }
}
