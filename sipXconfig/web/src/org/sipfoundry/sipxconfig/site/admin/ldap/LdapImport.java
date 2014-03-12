/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.ldap;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapImportTrigger;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.common.CronSchedule;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class LdapImport extends BaseComponent implements PageBeginRenderListener {

    private static final Log LOG = LogFactory.getLog(LdapImport.class);
    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract LdapImportManager getLdapImportManager();

    public abstract LdapManager getLdapManager();

    @InjectObject("spring:ldapImportTrigger")
    public abstract LdapImportTrigger getLdapImportTrigger();

    public abstract CronSchedule getSchedule();

    public abstract void setSchedule(CronSchedule schedule);

    @Parameter(required = true)
    public abstract int getCurrentConnectionId();

    public void pageBeginRender(PageEvent event) {
        if (getSchedule() == null) {
            setSchedule(getLdapManager().getSchedule(getCurrentConnectionId()));
        }
    }

    public void importLdap() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        if (!getLdapImportTrigger().isScheduledImportRunning()) {
            getLdapImportManager().insert(getCurrentConnectionId());
            getValidator().recordSuccess(getMessages().getMessage("msg.success"));
        } else {
            LOG.error("Cannot initiate import - a scheduled import is running");
            getValidator().record(new ValidatorException(getMessages().getMessage("msg.scheduled.import.running")));
        }
    }

    public IPage verifyLdap(IRequestCycle cycle) {
        LdapImportPreview ldapImportPreview = (LdapImportPreview) cycle.getPage(LdapImportPreview.PAGE);
        ldapImportPreview.setExample(null);
        ldapImportPreview.setCurrentConnectionId(getCurrentConnectionId());
        return ldapImportPreview;
    }

    public void applySchedule() {
        getLdapManager().setSchedule(getSchedule(), getCurrentConnectionId());
    }
}
