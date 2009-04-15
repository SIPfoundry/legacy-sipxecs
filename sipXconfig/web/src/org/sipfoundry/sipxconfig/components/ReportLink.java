/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;
import org.sipfoundry.sipxconfig.components.service.ReportService;
import org.sipfoundry.sipxconfig.components.service.ReportService.Info;

@ComponentClass(allowBody = true, allowInformalParameters = true)
public abstract class ReportLink extends BaseComponent {
    @InjectObject(value = "engine-service:" + ReportService.SERVICE_NAME)
    public abstract IEngineService getReportService();

    @Parameter(name = "reportPath", required = true)
    public abstract String getReportPath();

    @Parameter(name = "contentType", required = true)
    public abstract String getContentType();

    public String getLink() {
        Info info = new Info(getReportPath(), getContentType());
        ILink link = getReportService().getLink(true, info);
        return link.getAbsoluteURL();
    }
}
