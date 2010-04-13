/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.text.NumberFormat;
import java.util.Locale;

import org.sipfoundry.sipxconfig.admin.forwarding.Ring;

public class ForkQueueValue {
    private static final float MAX = 1.0f;
    private static final float MIN = 0.80f;
    private static final int MAX_FRAC = 3;
    private static final int MIN_FRAC = 1;

    private final NumberFormat m_format = NumberFormat.getInstance(Locale.ENGLISH);

    private float m_step;
    private float m_value = MAX;

    public ForkQueueValue(int sequenceCont) {
        m_format.setMaximumFractionDigits(MAX_FRAC);
        m_format.setMinimumFractionDigits(MIN_FRAC);
        m_step = (MAX - MIN) / (sequenceCont + 1);
    }

    /**
     * Use for serial forking
     */
    public String getSerial() {
        float next = m_value - m_step;
        if (next <= MIN) {
            throw new IllegalStateException();
        }
        m_value = next;
        return getParallel();
    }

    /**
     * Use for parallel forking.
     *
     * It is safe to call this function even if getNextValue has not been called first.
     */
    public String getParallel() {
        return "q=" + m_format.format(m_value);
    }

    public String getValue(Ring.Type type) {
        if (Ring.Type.IMMEDIATE.equals(type)) {
            return getParallel();
        }
        return getSerial();
    }
}
