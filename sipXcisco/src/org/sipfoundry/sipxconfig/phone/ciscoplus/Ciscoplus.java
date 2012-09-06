/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.ciscoplus;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.lang.time.DateUtils;
import org.sipfoundry.sipxconfig.registrar.RegistrationContext;
import org.sipfoundry.sipxconfig.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.device.RestartException;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;

/**
 * Support for ciscoplus 79xx
 */
public abstract class Ciscoplus extends Phone {

    public static final String PORT = "port";

    public static final String SIP = "sip";

    public static final String RESET_MESSAGE = "action=reset\r\nRegisterCallId={%s}\r\n";

    private RegistrationContext m_registrationContext;

    protected Ciscoplus() {
    }

    public int getMaxLineCount() {
        return getModel().getMaxLineCount();
    }

    @Override
    public void restart() {
        if (getLines().size() == 0) {
            throw new RestartException("&phone.line.not.valid");
        }
        Line line = getLines().get(0);
        List<RegistrationItem> registrations = m_registrationContext.getRegistrationsByUser(line.getUser());
        List<String> notifies = new ArrayList<String>();
        long currentTime = System.currentTimeMillis() / DateUtils.MILLIS_PER_SECOND;
        for (RegistrationItem registration : registrations) {
            if (registration.getInstrument().contains(getSerialNumber())) {
                long timeToExpire = registration.timeToExpireAsSeconds(currentTime);
                if (timeToExpire > 0) {
                    notifies.add(String.format(RESET_MESSAGE, registration.getRegCallId()));
                }
            }
        }
        boolean success = false;
        for (String notify : notifies) {
            try {
                getSipService().sendNotify(line.getAddrSpec(), "service-control", "text/plain", notify.getBytes());
                success = true;
            } catch (IllegalArgumentException iae) {
                // do nothing, success will be on false and error thrown
                continue;
            } catch (RuntimeException re) {
                // do nothing, success will be on false and error thrown
                continue;
            }
        }

        if (!success) {
            throw new RestartException("&phone.sip.exception");
        }
    }

    public void setRegistrationContext(RegistrationContext context) {
        m_registrationContext = context;
    }
}
