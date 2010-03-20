/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.text.DecimalFormat;

import org.apache.commons.lang.time.DateUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.RegistrationContext;
import org.sipfoundry.sipxconfig.admin.commserver.RegistrationMetrics;
import org.sipfoundry.sipxconfig.components.SipxBasePage;

/**
 * Displays active and expired registrations
 */
public abstract class Registrations extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/commserver/Registrations";

    @InjectObject(value = "spring:registrationContext")
    public abstract RegistrationContext getRegistrationContext();

    @Bean
    public abstract EvenOdd getRowClass();

    @Bean(initializer = "maximumFractionDigits=2,minimumFractionDigits=2")
    public abstract DecimalFormat getTwoDigitDecimal();

    @Persist(value = "session")
    public abstract boolean getDisplayPrimary();

    public abstract void setMetricsProperty(RegistrationMetrics registrationMetrics);

    public abstract RegistrationMetrics getMetricsProperty();

    public void pageBeginRender(PageEvent event_) {
        getMetrics();
    }

    /**
     * Retrieves registration metrics object. Can be called multiple times during rewind/render
     * and it'll lazily initialize registrations metrics only the first time it is called.
     *
     * Workaround for Tapestry 4.0 table model problem, in some case table model is retrieved
     * before pageBenginRender gets called
     *
     * @return properly initialized registration metrics object
     */
    public RegistrationMetrics getMetrics() {
        RegistrationMetrics metrics = getMetricsProperty();
        if (metrics != null) {
            return metrics;
        }
        long startRenderingTime = System.currentTimeMillis() / DateUtils.MILLIS_PER_SECOND;
        metrics = new RegistrationMetrics();
        metrics.setRegistrations(getRegistrationContext().getRegistrations());
        metrics.setStartTime(startRenderingTime);
        setMetricsProperty(metrics);
        return metrics;
    }


}
