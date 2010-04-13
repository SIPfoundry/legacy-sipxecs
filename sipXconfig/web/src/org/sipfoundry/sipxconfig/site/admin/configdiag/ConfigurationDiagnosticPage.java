/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.configdiag;

import java.text.DateFormat;
import java.text.Format;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnostic;
import org.sipfoundry.sipxconfig.admin.configdiag.ConfigurationDiagnosticContext;
import org.sipfoundry.sipxconfig.components.SipxBasePage;

public abstract class ConfigurationDiagnosticPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/configdiag/ConfigurationDiagnosticPage";

    @Asset("context:/WEB-INF/admin/configdiag/ConfigurationDiagnosticPage.script")
    public abstract IAsset getScript();

    @InjectObject(value = "spring:configurationDiagnosticContext")
    public abstract ConfigurationDiagnosticContext getConfigurationDiagnosticContext();

    @Bean
    public abstract EvenOdd getRowClass();

    public abstract int getCurrentRowId();

    public abstract List<ConfigurationDiagnostic> getConfigurationTests();

    public abstract void setConfigurationTests(List<ConfigurationDiagnostic> configurationTests);

    public abstract ConfigurationDiagnostic getConfigurationTest();

    public abstract Format getDateFormat();

    public abstract void setDateFormat(Format format);

    public void pageBeginRender(PageEvent event) {
        if (getConfigurationTests() != null) {
            return;
        }
        setConfigurationTests(getConfigurationDiagnosticContext().getConfigurationTests());
        Format dateFormat = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT,
                getLocale());
        setDateFormat(dateFormat);
    }

    public void runAllTests() {
        getConfigurationDiagnosticContext().runTests();
    }
}
