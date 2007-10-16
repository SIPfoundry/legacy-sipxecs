/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.monitoring.MRTGTarget;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringBean;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringUtil;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class StatisticsPage extends BasePage implements PageBeginRenderListener {
    public static final Object PAGE = "admin/StatisticsPage";

    private static final String SUMMARY_REPORT = "summary";

    private static final String DAILY = "daily";

    private static final String WEEKLY = "weekly";

    private static final String MONTHLY = "monthly";

    private static final String YEARLY = "yearly";

    @InjectObject(value = "spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    @InjectPage(value = MonitoringConfigurationPage.PAGE)
    public abstract MonitoringConfigurationPage getConfigurationPage();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract Object getAvailableReportsIndexItem();

    public abstract void setAvailableReportsIndexItem(Object index);

    public abstract int getReportIndex();

    public abstract void setReportIndex(int index);

    @Persist
    public abstract String getReportName();

    public abstract void setReportName(String reportName);

    public abstract void setHost(String host);

    public abstract String getHost();

    public abstract IPropertySelectionModel getHostModel();

    public abstract void setHostModel(IPropertySelectionModel model);

    public abstract Object getCurrentBean();

    public abstract void setCurrentBean(Object graph);

    public abstract int getIndex();

    public void showReport(String reportName, String host) {
        setReportName(reportName);
        setHost(host);
    }

    public IPage configure() {
        MonitoringConfigurationPage page = getConfigurationPage();
        page.setReturnPage(this);
        return page;
    }

    public void pageBeginRender(PageEvent event) {
        if (getHostModel() == null) {
            List<String> hosts = getMonitoringContext().getHosts();
            Collections.sort(hosts);
            StringPropertySelectionModel model = new StringPropertySelectionModel(hosts
                    .toArray(new String[hosts.size()]));
            setHostModel(model);
        }

        if (getHost() == null && getHostModel().getOptionCount() > 0) {
            setHost(getHostModel().getLabel(0));
        }

        String report = getReportName();
        if (report == null || !getMonitoringContext().getReports(getHost()).contains(report)) {
            setReportName(getMessages().getMessage(SUMMARY_REPORT));
        }
    }

    public List<String> getReports() {
        // generate the png files for the selected host
        getMonitoringContext().updateGraphs(getHost());

        // return reports for the given host and append 'summary' report
        List<String> reportNames = getMonitoringContext().getReports(getHost());
        List<String> reportsWithSummary = new ArrayList<String>();
        reportsWithSummary.add(getMessages().getMessage(SUMMARY_REPORT));
        for (String reportName : reportNames) {
            reportsWithSummary.add(reportName);
        }
        return reportsWithSummary;
    }

    public List<MonitoringBean> getReportBeans() {
        List<MRTGTarget> targets = getMonitoringContext().getTargetsForHost(getHost());
        List<MonitoringBean> monitoringBeans = new ArrayList<MonitoringBean>();
        String mrtgWorkingDir = getMonitoringContext().getMrtgWorkingDir();
        // add all images for summary report
        if (getReportName().equalsIgnoreCase(getMessages().getMessage(SUMMARY_REPORT))) {
            for (MRTGTarget target : targets) {
                monitoringBeans.add(getMonitoringBean(target.getTitle(), target,
                        MonitoringUtil.DAY_SUMMARY_EXTENSION, mrtgWorkingDir));
            }
        } else {
            // add detailed statistics for report
            MRTGTarget target = getMonitoringContext().getMRTGTarget(getReportName(), getHost());

            monitoringBeans.add(getMonitoringBean(getMessages().getMessage(DAILY), target,
                    MonitoringUtil.DAY_EXTENSION, mrtgWorkingDir));
            monitoringBeans.add(getMonitoringBean(getMessages().getMessage(WEEKLY), target,
                    MonitoringUtil.WEEK_EXTENSION, mrtgWorkingDir));
            monitoringBeans.add(getMonitoringBean(getMessages().getMessage(MONTHLY), target,
                    MonitoringUtil.MONTH_EXTENSION, mrtgWorkingDir));
            monitoringBeans.add(getMonitoringBean(getMessages().getMessage(YEARLY), target,
                    MonitoringUtil.YEAR_EXTENSION, mrtgWorkingDir));

        }
        return monitoringBeans;
    }

    private MonitoringBean getMonitoringBean(String reportName, MRTGTarget target,
            String extension, String workingDir) {
        MonitoringBean bean = new MonitoringBean(reportName, getImageLink(target, extension,
                workingDir), getHtmlDetails(target, extension, workingDir));
        return bean;
    }

    private String getImageLink(MRTGTarget target, String extension, String mrtgWorkingDir) {
        String mrtgWorkingPath = mrtgWorkingDir + MonitoringUtil.FORWARD_SLASH;
        return mrtgWorkingPath + target.getId() + extension + MonitoringUtil.GRAPH_EXTENSION;
    }

    private String getHtmlDetails(MRTGTarget target, String extension, String mrtgWorkingDir) {
        return MonitoringUtil.getHtmlDetailsForGraph(target, extension, mrtgWorkingDir);
    }

    public boolean isSummaryReport() {
        return (getReportName().equals(getMessages().getMessage(SUMMARY_REPORT)));
    }

    public boolean isEvenIndex() {
        return (getIndex() % 2 == 0);
    }
}
