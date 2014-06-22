/**
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.api.impl;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.text.DateFormat;
import java.text.ParseException;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import javax.servlet.http.HttpServletRequest;
import javax.ws.rs.WebApplicationException;
import javax.ws.rs.core.Response;
import javax.ws.rs.core.Response.ResponseBuilder;
import javax.ws.rs.core.Response.Status;
import javax.ws.rs.core.StreamingOutput;

import net.sf.jasperreports.engine.JRDataSource;
import net.sf.jasperreports.engine.JRException;
import net.sf.jasperreports.engine.JasperReport;
import net.sf.jasperreports.engine.util.JRLoader;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.api.CdrApi;
import org.sipfoundry.sipxconfig.api.model.CdrList;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.sipfoundry.sipxconfig.cdr.CdrSearch;
import org.sipfoundry.sipxconfig.cdr.CdrSettings;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.springframework.ui.jasperreports.JasperReportsUtils;

public class CdrApiImpl extends BaseServiceApiImpl implements CdrApi {
    private static final Log LOG = LogFactory.getLog(CdrApiImpl.class);
    private CdrManager m_cdrManager;
    private CoreContext m_coreContext;

    public void setCdrManager(CdrManager manager) {
        m_cdrManager = manager;
    }

    @Override
    protected PersistableSettings getSettings() {
        return m_cdrManager.getSettings();
    }

    @Override
    protected void saveSettings(PersistableSettings settings) {
        m_cdrManager.saveSettings((CdrSettings) settings);
    }

    @Override
    public Response getActiveCdrs(HttpServletRequest request) {
        return Response.ok().entity(CdrList.convertCdrList(m_cdrManager.getActiveCalls(), request.getLocale()))
                .build();
    }

    @Override
    public Response getCdrHistory(String fromDate, String toDate, String from, String to, Integer limit,
            Integer offset, String orderBy, HttpServletRequest request) {
        return Response
                .ok()
                .entity(CdrList.convertCdrList(getCdrs(fromDate, toDate, from, to, limit, offset, orderBy, null),
                        request.getLocale())).build();
    }

    @Override
    public Response getUserActiveCdrs(String userId, HttpServletRequest request) {
        User user = getUserByIdOrUserName(userId);
        if (user != null) {
            try {
                List<Cdr> cdrs = m_cdrManager.getActiveCallsREST(user);
                return Response.ok().entity(CdrList.convertCdrList(cdrs, request.getLocale())).build();
            } catch (IOException ex) {
                return Response.serverError().entity(userId).build();
            }
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getUserCdrHistory(String userId, String fromDate, String toDate, String from, String to,
            Integer limit, Integer offset, String orderBy, HttpServletRequest request) {
        User user = getUserByIdOrUserName(userId);
        if (user != null) {
            return Response
                    .ok()
                    .entity(CdrList.convertCdrList(
                            getCdrs(fromDate, toDate, from, to, limit, offset, orderBy, user), request.getLocale()))
                    .build();
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    @Override
    public Response getCdrReports(HttpServletRequest request) {
        return Response.ok().entity(getFileList()).build();
    }

    @Override
    public Response getCdrTableReports(String fromDate, String toDate, String from, String to, Integer limit,
            Integer offset, String orderBy, HttpServletRequest request) {
        Date start = getDate(fromDate, RequestUtils.getDefaultStartTime());
        Date end = getDate(toDate, RequestUtils.getDefaultEndTime());
        List<Cdr> cdrs = getCdrs(start, end, from, to, limit, offset, orderBy, null);
        return generateReport(CdrList.convertCdrList(cdrs, request.getLocale()), start, end, request.getLocale());
    }

    @Override
    public Response getUserCdrTableReports(String userId, String fromDate, String toDate, String from, String to,
            Integer limit, Integer offset, String orderBy, HttpServletRequest request) {
        User user = getUserByIdOrUserName(userId);
        if (user != null) {
            Date start = getDate(fromDate, RequestUtils.getDefaultStartTime());
            Date end = getDate(toDate, RequestUtils.getDefaultEndTime());
            List<Cdr> cdrs = getCdrs(start, end, from, to, limit, offset, orderBy, user);
            return generateReport(CdrList.convertCdrList(cdrs, request.getLocale()), start, end, request.getLocale());
        }
        return Response.status(Status.NOT_FOUND).build();
    }

    private Response generateReport(final CdrList cdrList, final Date fromDate, final Date toDate, Locale locale) {
        final DateFormat dateFormat = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT, locale);
        StreamingOutput stream = new StreamingOutput() {
            @Override
            public void write(OutputStream os) throws IOException, WebApplicationException {
                try {
                    JasperReport report = (JasperReport) JRLoader.loadObject(new File(getPath(),
                            "cdr-table-report.jasper"));
                    JRDataSource source = JasperReportsUtils.convertReportData(cdrList.getCdrs());
                    Map<String, Object> parameters = new HashMap<String, Object>();
                    parameters.put("name", "CDR Table Report");
                    parameters.put("start", dateFormat.format(fromDate));
                    parameters.put("end", dateFormat.format(toDate));
                    JasperReportsUtils.renderAsPdf(report, parameters, source, os);
                } catch (JRException ex) {
                    LOG.error("Failed to generate report " + ex.getMessage());
                    throw new WebApplicationException(Response.serverError().entity("Failed to gerenate report")
                            .build());
                }
            }
        };
        ResponseBuilder responseBuilder = Response.ok(stream, "application/pdf");
        responseBuilder.header(ResponseUtils.CONTENT_DISPOSITION, "attachment; filename=cdr-table-report.pdf");
        return responseBuilder.build();
    }

    private User getUserByIdOrUserName(String id) {
        User user = null;
        try {
            int userId = Integer.parseInt(id);
            user = m_coreContext.getUser(userId);
        } catch (NumberFormatException e) {
            user = null;
        }
        if (user == null) {
            user = m_coreContext.loadUserByUserNameOrAlias(id);
        }
        return user;
    }

    private Date getDate(String date, Date defaultDate) {
        try {
            if (date != null) {
                return RequestUtils.DATE_FORMAT.parse(date);
            }
        } catch (ParseException ex) {
            throw new WebApplicationException(Response.status(Status.BAD_REQUEST).entity("Bad date format").build());
        }
        return defaultDate;
    }

    private List<Cdr> getCdrs(Date start, Date end, String from, String to, Integer limit, Integer offset,
            String orderBy, User user) {

        CdrSearch search = getSearch(from, to, orderBy);
        return m_cdrManager.getCdrs(start, end, search, user, getWithDefaultValue(limit),
                getWithDefaultValue(offset));
    }

    private static Integer getWithDefaultValue(Integer value) {
        if (value == null) {
            return 0;
        }
        return value;
    }

    private List<Cdr> getCdrs(String fromDate, String toDate, String from, String to, Integer limit, Integer offset,
            String orderBy, User user) {
        Date start = getDate(fromDate, RequestUtils.getDefaultStartTime());
        Date end = getDate(toDate, RequestUtils.getDefaultEndTime());
        return getCdrs(start, end, from, to, limit, offset, orderBy, user);
    }

    private CdrSearch getSearch(String from, String to, String orderBy) {
        CdrSearch search = new CdrSearch();
        if (StringUtils.isNotBlank(from) && StringUtils.isNotBlank(to) && StringUtils.equals(from, to)) {
            search.setMode(CdrSearch.Mode.ANY);
            search.setTerm(new String[] {
                from
            });
        } else if (StringUtils.isNotBlank(from)) {
            search.setMode(CdrSearch.Mode.CALLER);
            search.setTerm(new String[] {
                from
            });
        } else if (StringUtils.isNotBlank(to)) {
            search.setMode(CdrSearch.Mode.CALLEE);
            search.setTerm(new String[] {
                to
            });
        }

        if (orderBy != null) {
            if (!CdrSearch.ORDER_TO_COLUMN.containsKey(orderBy)) {
                throw new WebApplicationException(Response.status(Status.BAD_REQUEST)
                        .entity("Unknown order by criteria").build());
            }
            search.setOrder(orderBy, true);
        }
        return search;
    }

    public void setCoreContext(CoreContext context) {
        m_coreContext = context;
    }
}
