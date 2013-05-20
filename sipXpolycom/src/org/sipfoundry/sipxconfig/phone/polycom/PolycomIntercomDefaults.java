/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import org.sipfoundry.sipxconfig.intercom.Intercom;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.BeanValueStorage;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

/**
 * Polycom allows many ring configurations to be set up using settings named
 * volpProt.SIP.alertInfo.x.value where x = 1, 2, ... n.
 *
 * For now x = 1, always, because we only provide a single ring configuration.
 *
 * Similarly there can be many ring classes. We rely on the default ring classes defined in
 * sip-2.0.cfg.vm rather than creating new ones.
 *
 * See sections 4.6.1.1.3.2 "Alert Information <alertInfo/>" and 4.6.1.7.2 "Ring type <ringType/>"
 * in the Polycom admin manual.
 *
 */
public class PolycomIntercomDefaults {
    static final String AUTO_ANSWER_RING_CLASS_32 = "3";
    static final String RING_ANSWER_RING_CLASS_32 = "4";
    static final String AUTO_ANSWER_RING_CLASS_40 = "autoAnswer";
    static final String RING_ANSWER_RING_CLASS_40 = "ringAutoAnswer";

    private Phone m_phone;

    public PolycomIntercomDefaults(Phone phone) {
        m_phone = phone;
    }

    @SettingEntry(path = "voIpProt.SIP/alertInfo/4/value")
    public String getAlertInfoValue() {
        Intercom intercom = getIntercom();
        if (intercom == null || !intercom.isEnabled()) {
            throw new BeanValueStorage.NoValueException();
        }
        return intercom.getCode();
    }

    @SettingEntry(path = "voIpProt.SIP/alertInfo/4/class")
    public String getAlertInfoClass() {
        Intercom intercom = getIntercom();
        if (intercom == null || !intercom.isEnabled()) {
            throw new BeanValueStorage.NoValueException();
        }
        // If the timeout is zero, then auto-answer. Otherwise ring for
        // the specified timeout before auto-answering.
        int timeout = intercom.getTimeout();
        String autoAnswer = AUTO_ANSWER_RING_CLASS_32;
        String ringAnswer = RING_ANSWER_RING_CLASS_32;
        if (m_phone.getDeviceVersion() == PolycomModel.VER_4_0_X
                || m_phone.getDeviceVersion() == PolycomModel.VER_4_1_X) {
            autoAnswer = AUTO_ANSWER_RING_CLASS_40;
        }
        if (m_phone.getDeviceVersion() == PolycomModel.VER_4_0_X
                || m_phone.getDeviceVersion() == PolycomModel.VER_4_1_X) {
            ringAnswer = RING_ANSWER_RING_CLASS_40;
        }
        return timeout > 0 ? ringAnswer : autoAnswer;
    }

    @SettingEntry(path = "se/ringType/RING_ANSWER/timeout")
    public int getRingAnswerTimeout() {
        Intercom intercom = getIntercom();
        if (intercom == null || !intercom.isEnabled() || intercom.getTimeout() <= 0) {
            throw new BeanValueStorage.NoValueException();
        }
        return intercom.getTimeout();
    }

    protected Intercom getIntercom() {
        PhoneContext context = m_phone.getPhoneContext();
        Intercom intercom = context.getIntercomForPhone(m_phone);
        return intercom;
    }
}
