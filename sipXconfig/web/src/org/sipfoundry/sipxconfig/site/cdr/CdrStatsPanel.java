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

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Date;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.sipfoundry.sipxconfig.cdr.CdrSearch;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class CdrStatsPanel extends BaseComponent {
    private static final Log LOG = LogFactory.getLog(CdrStatsPanel.class);

    @InjectObject(value = "spring:cdrManager")
    public abstract CdrManager getCdrManager();

    @InjectObject(value = "service:tapestry.globals.WebResponse")
    public abstract WebResponse getResponse();

    @Parameter
    public abstract Date getStartTime();

    @Parameter
    public abstract Date getEndTime();

    @Parameter
    public abstract User getUser();

    @Parameter
    public abstract CdrSearch getCdrSearch();

    public CdrTableModel getCdrTableModel() {
        CdrTableModel tableModel = new CdrTableModel(getCdrManager());
        tableModel.setFrom(getStartTime());
        tableModel.setTo(getEndTime());
        tableModel.setCdrSearch(getCdrSearch());
        tableModel.setUser(getUser());
        return tableModel;
    }

    public void export() {
        try {
            PrintWriter writer = TapestryUtils.getCsvExportWriter(getResponse(), "cdrs.csv");
            getCdrManager().dumpCdrs(writer, getStartTime(), getEndTime(),
                    getCdrSearch(), getUser());
            writer.close();
        } catch (IOException e) {
            LOG.error("Error during CDR export", e);
        }
    }
}
