/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import java.util.Collections;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Message;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ActiveCallsPanel extends BaseComponent {
    public static final Log LOG = LogFactory.getLog(ActiveCallsPanel.class);

    @InjectObject(value = "spring:cdrManager")
    public abstract CdrManager getCdrManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract List<Cdr> getActiveCalls();

    public abstract void setActiveCalls(List<Cdr> cdrs);

    @Message
    public abstract String getError();

    // FIXME: it would be much better if we can set activeCalls property in prepare for render,
    // but for some reason (Tapestry bug?) prepareForTender gets called to late during Ajax
    // triggered page interactions
    public List<Cdr> getSource() {
        List<Cdr> activeCalls = getActiveCalls();
        if (activeCalls == null) {
            try {
                activeCalls = getCdrManager().getActiveCalls();
            } catch (UserException e) {
                LOG.error("Cannot connect to CDR agent", e.getCause());
                //FIXME: validator errors are not visible - they are logged after @ErrorMessage is rendered
                getValidator().record(getError(), ValidationConstraint.CONSISTENCY);
                activeCalls = Collections.emptyList();
            }
            setActiveCalls(activeCalls);
        }
        return activeCalls;
    }
}
