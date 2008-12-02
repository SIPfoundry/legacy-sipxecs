/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import java.awt.Color;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

import org.apache.commons.collections.Bag;
import org.apache.commons.collections.bag.HashBag;
import org.apache.commons.lang.time.DateUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.data.general.DefaultPieDataset;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.cdr.CdrGraphBean;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl;
import org.sipfoundry.sipxconfig.cdr.CdrMinutesGraphBean;
import org.sipfoundry.sipxconfig.cdr.CdrSearch;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.jasperreports.JasperReportContext;
import org.sipfoundry.sipxconfig.site.cdr.decorators.CdrCallerDecorator;
import org.sipfoundry.sipxconfig.site.cdr.decorators.CdrDecorator;

public abstract class CdrReports extends BaseComponent implements PageBeginRenderListener {
    public static final String PAGE = "cdr/CdrReports";

    private static final String TITLE_TABLE_REPORT = "tableReportTitle";

    private static final String TITLE_EXTENSION_REPORT = "extensionReportTitle";

    private static final String TITLE_ACTIVE_CALLERS_GRAPH = "activeCallersGraphTitle";

    private static final String TITLE_ACTIVE_RECEIVERS_GRAPH = "activeReceiversGraphTitle";

    private static final String TITLE_MINUTES_OUTGOING_EXTENSION_GRAPH = "minutesOutgoingGraphTitle";

    private static final String TITLE_TERMINATION_CALLS_PIE = "terminationCallsPieTitle";

    private static final String TITLE_CDR_QUERY_REPORT = "cdrQueryReportTitle";

    private static final String TITLE_CONFIG_QUERY_REPORT = "configQueryReportTitle";

    private static final String EMPTY_TITLE = "";

    @InjectObject(value = "spring:jasperReportContextImpl")
    public abstract JasperReportContext getJaperReportContext();

    @InjectObject(value = "spring:cdrManagerImpl")
    public abstract CdrManagerImpl getCdrManager();

    @Persist
    @InitialValue(value = "literal:active")
    public abstract String getTab();

    @Persist
    public abstract Date getStartTime();

    public abstract void setStartTime(Date startTime);

    @Persist
    public abstract Date getEndTime();

    public abstract void setEndTime(Date endTime);

    @Parameter
    public abstract User getUser();

    @Persist
    public abstract CdrSearch getCdrSearch();

    public abstract void setCdrSearch(CdrSearch cdrSearch);

    @Persist
    public abstract List< ? > getCdrTableData();

    public abstract void setCdrTableData(List< ? > data);

    @Persist
    public abstract Map<String, String> getCdrTableParameters();

    public abstract void setCdrTableParameters(Map<String, String> parameters);

    @Persist
    public abstract List< ? > getCdrExtensionData();

    public abstract void setCdrExtensionData(List< ? > data);

    @Persist
    public abstract Map<String, String> getCdrExtensionParameters();

    public abstract void setCdrExtensionParameters(Map<String, String> parameters);

    @Persist
    public abstract List< ? > getCdrActiveCallersData();

    public abstract void setCdrActiveCallersData(List< ? > data);

    @Persist
    public abstract Map<String, Object> getCdrActiveCallersParameters();

    public abstract void setCdrActiveCallersParameters(Map<String, Object> parameters);

    @Persist
    public abstract List< ? > getCdrActiveReceiversData();

    public abstract void setCdrActiveReceiversData(List< ? > data);

    @Persist
    public abstract Map<String, Object> getCdrActiveReceiversParameters();

    public abstract void setCdrActiveReceiversParameters(Map<String, Object> parameters);

    @Persist
    public abstract List< ? > getCdrMinutesOutgoingExtensionData();

    public abstract void setCdrMinutesOutgoingExtensionData(List< ? > data);

    @Persist
    public abstract Map<String, Object> getCdrMinutesOutgoingExtensionParameters();

    public abstract void setCdrMinutesOutgoingExtensionParameters(Map<String, Object> parameters);

    @Persist
    public abstract List< ? > getCdrTerminationCallsData();

    public abstract void setCdrTerminationCallsData(List< ? > data);

    @Persist
    public abstract Map<String, Object> getCdrTerminationCallsParameters();

    public abstract void setCdrTerminationCallsParameters(Map<String, Object> parameters);

    @Persist
    public abstract Map<String, Object> getCdrQueryReportParameters();

    public abstract void setCdrQueryReportParameters(Map<String, Object> parameters);

    @Persist
    public abstract Map<String, Object> getConfigQueryReportParameters();

    public abstract void setConfigQueryReportParameters(Map<String, Object> parameters);

    public void pageBeginRender(PageEvent event_) {
        if (getEndTime() == null) {
            setEndTime(getDefaultEndTime());
        }
        if (getStartTime() == null) {
            Date startTime = getDefaultStartTime(getEndTime());
            setStartTime(startTime);
        }
        if (getCdrSearch() == null) {
            setCdrSearch(new CdrSearch());
        }

        List<Cdr> cdrs = getCdrManager().getCdrs(getStartTime(), getEndTime(), getCdrSearch(),
                getUser());
        List<CdrDecorator> cdrsData = new ArrayList<CdrDecorator>();
        List<CdrCallerDecorator> cdrsCaller = new ArrayList<CdrCallerDecorator>();
        Locale locale = getPage().getLocale();
        for (Cdr cdr : cdrs) {
            CdrDecorator cdrDecorator = new CdrDecorator(cdr, locale, getMessages());
            cdrsData.add(cdrDecorator);
            CdrCallerDecorator cdrCallerDecorator = new CdrCallerDecorator(cdr, locale,
                    getMessages());
            cdrsCaller.add(cdrCallerDecorator);
        }

        // Set data for CDR table report
        setCdrTableData(cdrsData);

        // Set data for CDR extension report
        Collections.sort(cdrsCaller);
        setCdrExtensionData(cdrsCaller);

        // Set data for CDR most active callers and receivers graphs
        Bag bagCallers = new HashBag();
        Bag bagReceivers = new HashBag();
        for (Cdr cdr : cdrs) {
            bagCallers.add(cdr.getCaller());
            bagReceivers.add(cdr.getCallee());
        }
        List<CdrGraphBean> activeCallers = mostActiveExtensions(bagCallers);
        List<CdrGraphBean> activeReceivers = mostActiveExtensions(bagReceivers);
        setCdrActiveCallersData(activeCallers);
        setCdrActiveReceiversData(activeReceivers);

        // Set data for CDR minutes of outgoing calls per extension graph
        List<CdrMinutesGraphBean> minutesOutgoingCalls = new ArrayList<CdrMinutesGraphBean>();
        Map<String, CdrMinutesGraphBean> outgoingCalls = new HashMap<String, CdrMinutesGraphBean>();
        for (Cdr cdr : cdrs) {
            String extension = cdr.getCaller();
            CdrMinutesGraphBean bean = outgoingCalls.get(extension);
            if (bean == null) {
                outgoingCalls.put(extension,
                        new CdrMinutesGraphBean(extension, cdr.getDuration()));
            } else {
                bean.setMinutes((bean.getMinutes() + cdr.getDuration()));
            }
        }
        minutesOutgoingCalls.addAll(outgoingCalls.values());
        Collections.sort(minutesOutgoingCalls, Collections.reverseOrder());
        if (minutesOutgoingCalls.size() > 10) {
            minutesOutgoingCalls = minutesOutgoingCalls.subList(0, 10);
        }
        setCdrMinutesOutgoingExtensionData(minutesOutgoingCalls);

        // Set data for CDR termination calls pie
        List<CdrGraphBean> terminationCalls = new ArrayList<CdrGraphBean>();
        Bag terminationCallsBag = new HashBag();
        for (Cdr cdr : cdrs) {
            CdrDecorator cdrDecorator = new CdrDecorator(cdr, locale, getMessages());
            terminationCallsBag.add(cdrDecorator.getTermination());
        }
        Set uniqueSetTermination = terminationCallsBag.uniqueSet();
        for (Object key : uniqueSetTermination) {
            CdrGraphBean bean = new CdrGraphBean((String) key, terminationCallsBag.getCount(key));
            terminationCalls.add(bean);
        }
        setCdrTerminationCallsData(terminationCalls);

        Map<String, String> mapTableReport = new HashMap<String, String>();
        Map<String, String> mapExtensionReport = new HashMap<String, String>();
        Map<String, Object> mapActiveCallersGraph = new HashMap<String, Object>();
        Map<String, Object> mapActiveReceiversGraph = new HashMap<String, Object>();
        Map<String, Object> mapMinutesOutgoingCallsGraph = new HashMap<String, Object>();
        Map<String, Object> mapTerminationCallsPie = new HashMap<String, Object>();
        if (cdrs.size() > 0) {
            mapTableReport.put(TITLE_TABLE_REPORT, getMessages().getMessage("report.cdrTable"));
            mapExtensionReport.put(TITLE_EXTENSION_REPORT, getMessages().getMessage(
                    "report.cdrExtension"));
            mapActiveCallersGraph.put(TITLE_ACTIVE_CALLERS_GRAPH, getMessages().getMessage(
                    "report.cdrActiveCallers"));
            mapActiveReceiversGraph.put(TITLE_ACTIVE_RECEIVERS_GRAPH, getMessages().getMessage(
                    "report.cdrActiveReceivers"));
            mapMinutesOutgoingCallsGraph.put(TITLE_MINUTES_OUTGOING_EXTENSION_GRAPH,
                    getMessages().getMessage("report.cdrMinutesOutgoingExtension"));
            mapTerminationCallsPie.put(TITLE_TERMINATION_CALLS_PIE, getMessages().getMessage(
                    "report.cdrTerminationCalls"));
        } else {
            String emptyData = getMessages().getMessage("report.emptyData");
            mapTableReport.put(TITLE_TABLE_REPORT, emptyData);
            mapExtensionReport.put(TITLE_EXTENSION_REPORT, emptyData);
            mapActiveCallersGraph.put(TITLE_ACTIVE_CALLERS_GRAPH, emptyData);
            mapActiveReceiversGraph.put(TITLE_ACTIVE_RECEIVERS_GRAPH, emptyData);
            mapMinutesOutgoingCallsGraph.put(TITLE_MINUTES_OUTGOING_EXTENSION_GRAPH, emptyData);
            mapTerminationCallsPie.put(TITLE_TERMINATION_CALLS_PIE, emptyData);
        }

        // Set parameters for CDR table report
        setCdrTableParameters(mapTableReport);

        // Set parameters for CDR extension report
        setCdrExtensionParameters(mapExtensionReport);

        // Set parameters for CDR most active callers graph
        String xAxis = getMessages().getMessage("report.cdrActiveExtensions.xAxis");
        String yAxis = getMessages().getMessage("report.cdrActiveCallers.yAxis");
        mapActiveCallersGraph.put("callersChart", createExtensionsChartImage(activeCallers,
                xAxis, yAxis));
        setCdrActiveCallersParameters(mapActiveCallersGraph);

        // Set parameters for CDR most active receivers graph
        yAxis = getMessages().getMessage("report.cdrActiveReceivers.yAxis");
        mapActiveReceiversGraph.put("receiversChart", createExtensionsChartImage(activeReceivers,
                xAxis, yAxis));
        setCdrActiveReceiversParameters(mapActiveReceiversGraph);

        // Set parameters for CDR Graph minutes of outgoing calls per extension graph
        yAxis = getMessages().getMessage("report.cdrMinutesOutgoingExtension.yAxis");
        mapMinutesOutgoingCallsGraph.put("minutesOutgoingExtensionChart",
                createMinutesOutgoingCallsChartImage(minutesOutgoingCalls, xAxis, yAxis));
        setCdrMinutesOutgoingExtensionParameters(mapMinutesOutgoingCallsGraph);

        // Set parameters for CDR termination calls pie
        mapTerminationCallsPie.put("terminationCallsPieImage",
                createTerminationCallsPieImage(terminationCalls));
        setCdrTerminationCallsParameters(mapTerminationCallsPie);

        // Set parameters for SIPXCDR query report
        Map<String, Object> mapCdrQueryReport = new HashMap<String, Object>();
        mapCdrQueryReport.put(TITLE_CDR_QUERY_REPORT, getMessages().getMessage(
                "report.cdrQueryReport"));
        setCdrQueryReportParameters(mapCdrQueryReport);

        // Set parameters for SIPXCONFIG query report
        Map<String, Object> mapConfigQueryReport = new HashMap<String, Object>();
        mapConfigQueryReport.put(TITLE_CONFIG_QUERY_REPORT, getMessages().getMessage(
                "report.configQueryReport"));
        setConfigQueryReportParameters(mapConfigQueryReport);
    }

    /**
     * By default set start at next midnight
     */
    public static Date getDefaultEndTime() {
        Calendar now = Calendar.getInstance();
        now.add(Calendar.DAY_OF_MONTH, 1);
        Calendar end = DateUtils.truncate(now, Calendar.DAY_OF_MONTH);
        return end.getTime();
    }

    /**
     * start a day before end time
     */
    public static Date getDefaultStartTime(Date endTime) {
        Calendar then = Calendar.getInstance();
        then.setTime(endTime);
        then.add(Calendar.DAY_OF_MONTH, -1);
        return then.getTime();
    }

    private List<CdrGraphBean> mostActiveExtensions(Bag bagExtensions) {
        List<CdrGraphBean> activeExtensions = new ArrayList<CdrGraphBean>();
        Set uniqueSetCallers = bagExtensions.uniqueSet();
        for (Object key : uniqueSetCallers) {
            CdrGraphBean bean = new CdrGraphBean((String) key, bagExtensions.getCount(key));
            activeExtensions.add(bean);
        }
        Collections.sort(activeExtensions, Collections.reverseOrder());
        if (activeExtensions.size() > 10) {
            activeExtensions = activeExtensions.subList(0, 10);
        }

        return activeExtensions;
    }

    private static Image createExtensionsChartImage(List<CdrGraphBean> extensions,
            String xAxisLabel, String yAxisLabel) {
        // Create a dataset...
        DefaultCategoryDataset data = new DefaultCategoryDataset();

        // Fill dataset with beans data
        for (CdrGraphBean extension : extensions) {
            data.setValue(extension.getCount(), extension.getKey(), extension.getKey());
        }

        // Create a chart with the dataset
        JFreeChart barChart = ChartFactory.createBarChart3D(EMPTY_TITLE, xAxisLabel, yAxisLabel,
                data, PlotOrientation.VERTICAL, true, true, true);
        barChart.setBackgroundPaint(Color.lightGray);
        barChart.getTitle().setPaint(Color.BLACK);
        CategoryPlot p = barChart.getCategoryPlot();
        p.setRangeGridlinePaint(Color.red);

        // Create and return the image with the size specified in the XML design
        return barChart.createBufferedImage(500, 220, BufferedImage.TYPE_INT_RGB, null);
    }

    private static Image createMinutesOutgoingCallsChartImage(
            List<CdrMinutesGraphBean> minutesOutgoingCalls, String xAxisLabel, String yAxisLabel) {
        // Create a dataset...
        DefaultCategoryDataset data = new DefaultCategoryDataset();

        // Fill dataset with beans data
        for (CdrMinutesGraphBean minutesOutgoingCall : minutesOutgoingCalls) {
            data.setValue(minutesOutgoingCall.getMinutes() / 60000, minutesOutgoingCall
                    .getExtension(), minutesOutgoingCall.getExtension());
        }

        // Create a chart with the dataset
        JFreeChart barChart = ChartFactory.createBarChart3D(EMPTY_TITLE, xAxisLabel, yAxisLabel,
                data, PlotOrientation.VERTICAL, true, true, true);
        barChart.setBackgroundPaint(Color.lightGray);
        barChart.getTitle().setPaint(Color.BLACK);
        CategoryPlot p = barChart.getCategoryPlot();
        p.setRangeGridlinePaint(Color.red);

        // Create and return the image with the size specified in the XML design
        return barChart.createBufferedImage(500, 220, BufferedImage.TYPE_INT_RGB, null);
    }

    private static Image createTerminationCallsPieImage(List<CdrGraphBean> beans) {
        // Create a dataset
        DefaultPieDataset data = new DefaultPieDataset();

        // Fill dataset with beans data
        for (CdrGraphBean terminationCall : beans) {
            data.setValue(terminationCall.getKey(), terminationCall.getCount());
        }

        // Create a chart with the dataset
        JFreeChart chart = ChartFactory.createPieChart(EMPTY_TITLE, data, true, true, true);
        chart.setBackgroundPaint(Color.lightGray);
        chart.getTitle().setPaint(Color.BLACK);

        // Create and return the image
        return chart.createBufferedImage(500, 220, BufferedImage.TYPE_INT_RGB, null);
    }
}
