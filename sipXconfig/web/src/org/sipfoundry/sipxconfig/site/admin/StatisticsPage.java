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
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.monitoring.MRTGTarget;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringBean;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringUtil;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.site.admin.commserver.EditLocationPage;
import org.sipfoundry.sipxconfig.site.admin.commserver.LocationsPage;

public abstract class StatisticsPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/StatisticsPage";

    private static final String SUMMARY_REPORT = "target.summary";

    private static final String DAILY = "daily";

    private static final String WEEKLY = "weekly";

    private static final String MONTHLY = "monthly";

    private static final String YEARLY = "yearly";

    private static final String LABEL = "label.";

    @InjectObject("spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectPage(LocationsPage.PAGE)
    public abstract LocationsPage getLocationsPage();

    @InjectPage(EditLocationPage.PAGE)
    public abstract EditLocationPage getEditLocationPage();

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

    @Persist
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

    public IPage configureTargets() {
        EditLocationPage page = getEditLocationPage();
        page.setLocationId(getLocationsManager().getLocationByFqdn(getHost()).getId());
        page.setTab(EditLocationPage.MONITOR_TAB);
        page.setReturnPage(this);
        return page;
    }

    public void pageBeginRender(PageEvent event) {
        if (getHostModel() == null) {
            List<String> hosts = getMonitoringContext().getAvailableHosts();
            Collections.sort(hosts);
            StringPropertySelectionModel model = new StringPropertySelectionModel(hosts
                    .toArray(new String[hosts.size()]));
            setHostModel(model);
            if (!hosts.contains(getHost()) && getHostModel().getOptionCount() > 0) {
                setHost(getHostModel().getLabel(0));
            }
        }

        if (getHost() == null && getHostModel().getOptionCount() > 0) {
            setHost(getHostModel().getLabel(0));
        }

        String report = getReportName();
        if (report == null || !getMonitoringContext().getReports(getHost()).contains(report)) {
            setReportName(SUMMARY_REPORT);
        }
    }

    public List<String> getReports() {
        // generate the png files for the selected host
        getMonitoringContext().updateGraphs(getHost());

        // return reports for the given host and append 'summary' report
        List<String> reportNames = getMonitoringContext().getReports(getHost());
        reportNames.add(0, SUMMARY_REPORT);
        return reportNames;
    }

    public List<MonitoringBean> getReportBeans() {
        List<MRTGTarget> targets = getMonitoringContext().getTargetsForHost(getHost());
        List<MonitoringBean> monitoringBeans = new ArrayList<MonitoringBean>();
        String mrtgWorkingDir = getMonitoringContext().getMrtgWorkingDir();
        // add all images for summary report
        if (getReportName().equalsIgnoreCase(SUMMARY_REPORT)) {
            for (MRTGTarget target : targets) {
                monitoringBeans.add(getMonitoringBean(target.getTitle(), target,
                        MonitoringUtil.DAY_SUMMARY_EXTENSION, mrtgWorkingDir));
            }
        } else {
            // add detailed statistics for report
            MRTGTarget target = getMonitoringContext().getMRTGTarget(getReportName(), getHost());

            monitoringBeans.add(getMonitoringBean(DAILY, target,
                    MonitoringUtil.DAY_EXTENSION, mrtgWorkingDir));
            monitoringBeans.add(getMonitoringBean(WEEKLY, target,
                    MonitoringUtil.WEEK_EXTENSION, mrtgWorkingDir));
            monitoringBeans.add(getMonitoringBean(MONTHLY, target,
                    MonitoringUtil.MONTH_EXTENSION, mrtgWorkingDir));
            monitoringBeans.add(getMonitoringBean(YEARLY, target,
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
        String htmlDetails = MonitoringUtil.getHtmlDetailsForGraph(target, extension,
                mrtgWorkingDir);
        htmlDetails = htmlDetails.replace("Average", i18nGraphsDetails("average")).replace("Max",
                i18nGraphsDetails("max")).replace("Current", i18nGraphsDetails("current"))
                .replace("Used", i18nGraphsDetails("used")).replace("Processes",
                        i18nGraphsDetails("processes"))
                .replace("Free", i18nGraphsDetails("free")).replace("Total",
                        i18nGraphsDetails("total")).replace("In", i18nGraphsDetails("in"))
                .replace("Out", i18nGraphsDetails("out"));
        return htmlDetails;
    }

    private String i18nGraphsDetails(String label) {
        return getMessages().getMessage(LABEL + label);
    }

    public boolean isSummaryReport() {
        return getReportName().equals(SUMMARY_REPORT);
    }

    public boolean isEvenIndex() {
        return (getIndex() % 2 == 0);
    }

    public boolean isHostHasTargetsForMonitoring() {
        return getMonitoringContext().getHosts().contains(getHost());
    }
}
