/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel12x0;

import org.sipfoundry.sipxconfig.admin.intercom.Intercom;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.BeanValueStorage;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class Nortel12x0IntercomDefaults {

    private Phone m_phone;

    public Nortel12x0IntercomDefaults(Phone phone) {
        m_phone = phone;
    }

    @SettingEntry(path = "intercom/alertInfoSecret")
    public String getAlertInfoValue() {
        Intercom intercom = getIntercom();
        if (intercom == null || !intercom.isEnabled()) {
            throw new BeanValueStorage.NoValueException();
        }
        return intercom.getCode();
    }

    @SettingEntry(path = "intercom/intercomPrefixValue")
    public String getIntercomPrefixValue() {
        Intercom intercom = getIntercom();
        if (intercom == null || !intercom.isEnabled()) {
            throw new BeanValueStorage.NoValueException();
        }
        return intercom.getPrefix();
    }

    protected Intercom getIntercom() {
        PhoneContext context = m_phone.getPhoneContext();
        Intercom intercom = context.getIntercomForPhone(m_phone);
        return intercom;
    }
}
