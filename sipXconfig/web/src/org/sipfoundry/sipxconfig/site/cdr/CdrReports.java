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
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collection;
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
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.labels.StandardPieSectionLabelGenerator;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.PiePlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.data.general.DefaultKeyedValuesDataset;
import org.jfree.data.general.DefaultPieDataset;
import org.sipfoundry.sipxconfig.admin.dialplan.CallTag;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.cdr.CdrGraphBean;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl;
import org.sipfoundry.sipxconfig.cdr.CdrMinutesGraphBean;
import org.sipfoundry.sipxconfig.cdr.CdrSearch;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ReportBean;
import org.sipfoundry.sipxconfig.components.ReportComponent;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.jasperreports.JasperReportContext;
import org.sipfoundry.sipxconfig.site.cdr.decorators.CdrCallDirectionDecorator;
import org.sipfoundry.sipxconfig.site.cdr.decorators.CdrCallLongDistanceDecorator;
import org.sipfoundry.sipxconfig.site.cdr.decorators.CdrCallerDecorator;
import org.sipfoundry.sipxconfig.site.cdr.decorators.CdrDecorator;

public abstract class CdrReports extends BaseComponent implements PageBeginRenderListener {
    public static final String PAGE = "cdr/CdrReports";

    private static final String TITLE_REPORT = "title";

    private static final String TITLE_STARTREPORT = "start";

    private static final String TITLE_ENDREPORT = "end";

    private static final String TITLE_TABLE_REPORT_KEY = "report.cdrTable";

    private static final String TITLE_LONGDISTANCE_REPORT_KEY = "report.cdrLongDistance";

    private static final String TITLE_CALLDIRECTION_REPORT_KEY = "report.cdrCallDirection";

    private static final String TITLE_EXTENSION_REPORT_KEY = "report.cdrExtension";

    private static final String TITLE_ACTIVE_CALLERS_GRAPH_KEY = "report.cdrActiveCallers";

    private static final String TITLE_ACTIVE_RECEIVERS_GRAPH_KEY = "report.cdrActiveReceivers";

    private static final String TITLE_MINUTES_OUTGOING_EXTENSION_GRAPH_KEY = "report.cdrMinutesOutgoingExtension";

    private static final String TITLE_TERMINATION_CALLS_PIE_KEY = "report.cdrTerminationCalls";

    private static final String TABLE_REPORT_NAME = "cdr-table-report";

    private static final String LONGDISTANCE_REPORT_NAME = "cdr-longdistance-report";

    private static final String CALLDIRECTION_REPORT_NAME = "cdr-calldirection-report";

    private static final String EXTENSION_REPORT_NAME = "cdr-extension-report";

    private static final String ACTIVE_CALLERS_GRAPH_NAME = "cdr-active-callers-graph";

    private static final String ACTIVE_RECEIVERS_GRAPH_NAME = "cdr-active-receivers-graph";

    private static final String MINUTES_OUTGOING_EXTENSION_GRAPH_NAME = "cdr-minutes-outgoing-graph";

    private static final String TERMINATION_CALLS_PIE_NAME = "cdr-termination-calls-pie";

    private static final String XLS_TOO_MANY_ROWS_EXCEPTION = "&error.xlsTooManyRows";

    private static final Log LOG = LogFactory.getLog(CdrReports.class);

    private static final String EMPTY_TITLE = "";

    private static final String PIECHART_SECTIONLABEL_FORMAT = "{0} = {1} ({2})";

    @InjectObject(value = "spring:jasperReportContextImpl")
    public abstract JasperReportContext getJasperReportContext();

    @InjectObject(value = "spring:cdrManagerImpl")
    public abstract CdrManagerImpl getCdrManager();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

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
    public abstract ReportBean getReportBean();

    public abstract void setReportBean(ReportBean bean);

    @Persist
    public abstract String getReportName();

    public abstract void setReportName(String reportName);

    @Persist
    public abstract String getReportLabel();

    public abstract void setReportLabel(String reportLabel);

    @Persist
    public abstract List< ? > getReportData();

    public abstract void setReportData(List< ? > data);

    @Persist
    public abstract Map<String, Object> getReportParameters();

    public abstract void setReportParameters(Map<String, Object> parameters);

    public abstract boolean isShowXlsLink();

    public abstract void setShowXlsLink(boolean show);

    public IPropertySelectionModel decorateModel(IPropertySelectionModel model) {
        return getTapestry().addExtraOption(model, getMessages(), "label.select");
    }

    public IPropertySelectionModel getReportModel() {
        Collection<ReportBean> beans = new ArrayList<ReportBean>();
        beans.add(new ReportBean(TABLE_REPORT_NAME, getMessages().getMessage(
                TITLE_TABLE_REPORT_KEY)));
        beans.add(new ReportBean(CALLDIRECTION_REPORT_NAME, getMessages().getMessage(
                TITLE_CALLDIRECTION_REPORT_KEY)));
        beans.add(new ReportBean(TERMINATION_CALLS_PIE_NAME, getMessages().getMessage(
                TITLE_TERMINATION_CALLS_PIE_KEY)));
        beans.add(new ReportBean(EXTENSION_REPORT_NAME, getMessages().getMessage(
                TITLE_EXTENSION_REPORT_KEY)));
        beans.add(new ReportBean(ACTIVE_CALLERS_GRAPH_NAME, getMessages().getMessage(
                TITLE_ACTIVE_CALLERS_GRAPH_KEY)));
        beans.add(new ReportBean(ACTIVE_RECEIVERS_GRAPH_NAME, getMessages().getMessage(
                TITLE_ACTIVE_RECEIVERS_GRAPH_KEY)));
        beans.add(new ReportBean(MINUTES_OUTGOING_EXTENSION_GRAPH_NAME, getMessages().getMessage(
                TITLE_MINUTES_OUTGOING_EXTENSION_GRAPH_KEY)));
        beans.add(new ReportBean(LONGDISTANCE_REPORT_NAME, getMessages().getMessage(
                TITLE_LONGDISTANCE_REPORT_KEY)));

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(beans);
        return model;
    }

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
    }

    public void formSubmit() {
        // Process form submission
        if (getReportBean() != null) {
            String reportName = getReportBean().getReportName();
            setReportName(reportName);
            setReportLabel(getReportBean().getReportLabel());
            computeReportData(reportName);

            // Generate reports
            ReportComponent reportComponent = (ReportComponent) getPage().getNestedComponent(
                    "cdrReports.report");
            try {
                reportComponent.generateReports();
                setShowXlsLink(true);
            } catch (IndexOutOfBoundsException iobEx) {
                LOG.error(XLS_TOO_MANY_ROWS_EXCEPTION, iobEx);
                throw new UserException(XLS_TOO_MANY_ROWS_EXCEPTION);
            }
        }
    }

    private void computeReportData(String reportName) {
        List<Cdr> cdrs = getCdrManager().getCdrs(getStartTime(), getEndTime(), getCdrSearch(),
                getUser());
        Locale locale = getPage().getLocale();
        Date startdate = getStartTime();
        Date enddate = getEndTime();
        SimpleDateFormat dateformat = new SimpleDateFormat("dd-MMM-yyyy kk:mm");
        Map<String, Object> mapParameters = new HashMap<String, Object>();

        String xAxis = getMessages().getMessage("report.cdrActiveExtensions.xAxis");
        String yAxis;
        if (reportName.equals(TABLE_REPORT_NAME)) {
            setReportData(getTableReportData(cdrs, locale));
            mapParameters.put(TITLE_REPORT, getMessages().getMessage(TITLE_TABLE_REPORT_KEY));
            mapParameters.put(TITLE_STARTREPORT, dateformat.format(startdate));
            mapParameters.put(TITLE_ENDREPORT, dateformat.format(enddate));
            setReportParameters(mapParameters);
            return;
        } else if (reportName.equals(LONGDISTANCE_REPORT_NAME)) {
            setReportData(getLongDistanceReportData(cdrs, locale));
            mapParameters.put(TITLE_REPORT, getMessages().getMessage(TITLE_LONGDISTANCE_REPORT_KEY));
            mapParameters.put(TITLE_STARTREPORT, dateformat.format(startdate));
            mapParameters.put(TITLE_ENDREPORT, dateformat.format(enddate));
            setReportParameters(mapParameters);
            return;
        } else if (reportName.equals(CALLDIRECTION_REPORT_NAME)) {
            List<CdrCallDirectionDecorator> data = getCallDirectionReportData(cdrs, locale);
            setReportData(data);
            mapParameters.put(TITLE_REPORT, getMessages().getMessage(TITLE_CALLDIRECTION_REPORT_KEY));
            mapParameters.put(TITLE_STARTREPORT, dateformat.format(startdate));
            mapParameters.put(TITLE_ENDREPORT, dateformat.format(enddate));
            setReportParameters(mapParameters);
            List<CdrGraphBean> gdata = getCallDirectionGraphData(data);
            mapParameters.put("calldirectionCallsPieImage", createCallDirectionCallsPieImage(gdata));
            setReportParameters(mapParameters);
            return;
        } else if (reportName.equals(EXTENSION_REPORT_NAME)) {
            setReportData(getExtensionReportData(cdrs, locale));
            mapParameters.put(TITLE_REPORT, getMessages().getMessage(TITLE_EXTENSION_REPORT_KEY));
            mapParameters.put(TITLE_STARTREPORT, dateformat.format(startdate));
            mapParameters.put(TITLE_ENDREPORT, dateformat.format(enddate));
            setReportParameters(mapParameters);
            return;
        } else if (reportName.equals(ACTIVE_CALLERS_GRAPH_NAME)) {
            List<CdrGraphBean> data = getActiveCallersData(cdrs);
            setReportData(data);
            mapParameters.put(TITLE_REPORT, getMessages().getMessage(
                    TITLE_ACTIVE_CALLERS_GRAPH_KEY));
            mapParameters.put(TITLE_STARTREPORT, dateformat.format(startdate));
            mapParameters.put(TITLE_ENDREPORT, dateformat.format(enddate));
            yAxis = getMessages().getMessage("report.cdrActiveCallers.yAxis");
            mapParameters.put("callersChart", createExtensionsChartImage(data, xAxis, yAxis));
            setReportParameters(mapParameters);
            return;
        } else if (reportName.equals(ACTIVE_RECEIVERS_GRAPH_NAME)) {
            List<CdrGraphBean> data = getActiveReceiversData(cdrs);
            setReportData(data);
            mapParameters.put(TITLE_REPORT, getMessages().getMessage(
                    TITLE_ACTIVE_RECEIVERS_GRAPH_KEY));
            mapParameters.put(TITLE_STARTREPORT, dateformat.format(startdate));
            mapParameters.put(TITLE_ENDREPORT, dateformat.format(enddate));
            yAxis = getMessages().getMessage("report.cdrActiveReceivers.yAxis");
            mapParameters.put("receiversChart", createExtensionsChartImage(data, xAxis, yAxis));
            setReportParameters(mapParameters);
            return;
        } else if (reportName.equals(MINUTES_OUTGOING_EXTENSION_GRAPH_NAME)) {
            List<CdrMinutesGraphBean> data = getOutgoingExtensionData(cdrs);
            setReportData(data);
            mapParameters.put(TITLE_REPORT, getMessages().getMessage(
                    TITLE_MINUTES_OUTGOING_EXTENSION_GRAPH_KEY));
            mapParameters.put(TITLE_STARTREPORT, dateformat.format(startdate));
            mapParameters.put(TITLE_ENDREPORT, dateformat.format(enddate));
            yAxis = getMessages().getMessage("report.cdrMinutesOutgoingExtension.yAxis");
            mapParameters.put("minutesOutgoingExtensionChart",
                    createMinutesOutgoingCallsChartImage(data, xAxis, yAxis));
            setReportParameters(mapParameters);
            return;
        } else if (reportName.equals(TERMINATION_CALLS_PIE_NAME)) {
            List<CdrGraphBean> data = getTerminationCallsData(cdrs, locale);
            setReportData(data);
            mapParameters.put(TITLE_REPORT, getMessages().getMessage(
                    TITLE_TERMINATION_CALLS_PIE_KEY));
            mapParameters.put(TITLE_STARTREPORT, dateformat.format(startdate));
            mapParameters.put(TITLE_ENDREPORT, dateformat.format(enddate));
            mapParameters.put("terminationCallsPieImage", createTerminationCallsPieImage(data));
            setReportParameters(mapParameters);
            return;
        }
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

    // Get data for CDR table report
    private List<CdrDecorator> getTableReportData(List<Cdr> cdrs, Locale locale) {
        List<CdrDecorator> cdrsData = new ArrayList<CdrDecorator>();
        for (Cdr cdr : cdrs) {
            CdrDecorator cdrDecorator = new CdrDecorator(cdr, locale, getMessages());
            cdrsData.add(cdrDecorator);
        }
        return cdrsData;
    }

    // Get data for CDR Long Distance report
    private List<CdrCallLongDistanceDecorator> getLongDistanceReportData(List<Cdr> cdrs, Locale locale) {
        String calleeroute;

        List<CdrCallLongDistanceDecorator> cdrsData = new ArrayList<CdrCallLongDistanceDecorator>();
        for (Cdr cdr : cdrs) {
            calleeroute = cdr.getCalleeRoute();
            if ((calleeroute != null)
                && (calleeroute.endsWith(CallTag.LD.toString())
                 || calleeroute.endsWith(CallTag.INTN.toString())
                 || calleeroute.endsWith(CallTag.REST.toString())
                 || calleeroute.endsWith(CallTag.TF.toString()))
            ) {
                CdrCallLongDistanceDecorator cdrLongDistanceDecorator =
                    new CdrCallLongDistanceDecorator(cdr, locale, getMessages());
                cdrsData.add(cdrLongDistanceDecorator);
            }
        }
        Collections.sort(cdrsData);
        return cdrsData;
    }

    // Get data for CDR call direction (Incoming/Outgoing/Tandem) report
    private List<CdrCallDirectionDecorator> getCallDirectionReportData(List<Cdr> cdrs, Locale locale) {
        List<CdrCallDirectionDecorator> cdrsCaller = new ArrayList<CdrCallDirectionDecorator>();
        for (Cdr cdr : cdrs) {
            CdrCallDirectionDecorator cdrCallDirectionDecorator = new CdrCallDirectionDecorator(cdr, locale,
                    getMessages());
            cdrsCaller.add(cdrCallDirectionDecorator);
        }
        Collections.sort(cdrsCaller);
        return cdrsCaller;
    }

    // Get graph data for CDR call direction pie.
    private List<CdrGraphBean> getCallDirectionGraphData(List<CdrCallDirectionDecorator> cdrdecorated) {
        List<CdrGraphBean> directionCalls = new ArrayList<CdrGraphBean>();
        Bag directionCallsBag = new HashBag();
        for (CdrCallDirectionDecorator cdr : cdrdecorated) {
            directionCallsBag.add(cdr.getCallDirection());
        }
        Set uniqueSetDirection = directionCallsBag.uniqueSet();
        for (Object key : uniqueSetDirection) {
            CdrGraphBean bean = new CdrGraphBean((String) key, directionCallsBag.getCount(key));
            directionCalls.add(bean);
        }
        return directionCalls;
    }

    // Get data for CDR extension report
    private List<CdrCallerDecorator> getExtensionReportData(List<Cdr> cdrs, Locale locale) {
        List<CdrCallerDecorator> cdrsCaller = new ArrayList<CdrCallerDecorator>();
        for (Cdr cdr : cdrs) {
            CdrCallerDecorator cdrCallerDecorator = new CdrCallerDecorator(cdr, locale,
                    getMessages());
            cdrsCaller.add(cdrCallerDecorator);
        }
        Collections.sort(cdrsCaller);
        return cdrsCaller;
    }

    // Get data for CDR most active callers graphs
    private List<CdrGraphBean> getActiveCallersData(List<Cdr> cdrs) {
        Bag bagCallers = new HashBag();
        for (Cdr cdr : cdrs) {
            bagCallers.add(cdr.getCaller());
        }
        List<CdrGraphBean> activeCallers = mostActiveExtensions(bagCallers);
        return activeCallers;
    }

    // Get data for CDR most active receivers graphs
    private List<CdrGraphBean> getActiveReceiversData(List<Cdr> cdrs) {
        Bag bagReceivers = new HashBag();
        for (Cdr cdr : cdrs) {
            bagReceivers.add(cdr.getCallee());
        }
        List<CdrGraphBean> activeReceivers = mostActiveExtensions(bagReceivers);
        return activeReceivers;
    }

    // Get data for CDR minutes of outgoing calls per extension graph
    private List<CdrMinutesGraphBean> getOutgoingExtensionData(List<Cdr> cdrs) {
        List<CdrMinutesGraphBean> minutesOutgoingCalls = new ArrayList<CdrMinutesGraphBean>();
        Map<String, CdrMinutesGraphBean> outgoingCalls = new HashMap<String, CdrMinutesGraphBean>();
        for (Cdr cdr : cdrs) {
            if (cdr.getDuration() > 0) {
                String extension = cdr.getCaller();
                CdrMinutesGraphBean bean = outgoingCalls.get(extension);
                if (bean == null) {
                    outgoingCalls.put(extension,
                            new CdrMinutesGraphBean(extension, (double) cdr.getDuration()));
                } else {
                    bean.setMinutes((bean.getMinutes() + cdr.getDuration()));
                }
            }
        }
        minutesOutgoingCalls.addAll(outgoingCalls.values());
        Collections.sort(minutesOutgoingCalls, Collections.reverseOrder());
        if (minutesOutgoingCalls.size() > 10) {
            minutesOutgoingCalls = minutesOutgoingCalls.subList(0, 10);
        }
        return minutesOutgoingCalls;
    }

    // Get data for CDR termination calls pie
    private List<CdrGraphBean> getTerminationCallsData(List<Cdr> cdrs, Locale locale) {
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
        return terminationCalls;
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

        PiePlot chartplot = (PiePlot) chart.getPlot();
        chartplot.setCircular(true);
        chartplot.setLabelGenerator(new StandardPieSectionLabelGenerator(PIECHART_SECTIONLABEL_FORMAT));
        // Create and return the image
        return chart.createBufferedImage(500, 220, BufferedImage.TYPE_INT_RGB, null);
    }


    private Image createCallDirectionCallsPieImage(List<CdrGraphBean> beans) {
        // Create a dataset
        DefaultKeyedValuesDataset data = new DefaultKeyedValuesDataset();

        // Fill dataset with beans data
        for (CdrGraphBean directionCall : beans) {
            data.setValue(directionCall.getKey(), directionCall.getCount());
        }

        // Create a chart with the dataset
        JFreeChart chart = ChartFactory.createPieChart(EMPTY_TITLE, data, true, true, false);
        chart.setBackgroundPaint(Color.lightGray);
        chart.setTitle("Summary - " + getMessages().getMessage(TITLE_CALLDIRECTION_REPORT_KEY));
        chart.getTitle().setPaint(Color.BLACK);

        PiePlot chartplot = (PiePlot) chart.getPlot();
        chartplot.setCircular(true);
        chartplot.setLabelGenerator(new StandardPieSectionLabelGenerator(PIECHART_SECTIONLABEL_FORMAT));

        // Create and return the image
        return chart.createBufferedImage(500, 220, BufferedImage.TYPE_INT_RGB, null);
    }
}
