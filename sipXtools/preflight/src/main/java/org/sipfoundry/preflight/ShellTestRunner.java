/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.io.InputStream;
import java.net.InetAddress;
import java.net.UnknownHostException;

import org.eclipse.swt.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

import org.sipfoundry.commons.dhcp.NetworkResources;
import org.sipfoundry.commons.util.JournalService;

import static org.sipfoundry.preflight.ResultCode.*;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class ShellTestRunner {
    private Display display;
    private JournalService journalService;

    // private final Font normalFont;
    // private final Font boldFont;
    private final Color black;
    private final Color white;
    private final Color magenta;
    private final Color red;
    private final Color green;
    private final Color blue;
    private final Color yellow;
    private final Image nullIcon;
    private final Image idleIcon;
    private final Image passedIcon;
    private final Image failedIcon;
    private final Image snafuIcon;
    private final Image warningIcon;
    private final Image skippedIcon;
    private final Image[] spinnerIcon;
    private final Table testSummaryTable;
    private final Test[] testTable;
    private boolean active;

    public static final int DHCP_TEST = 0;
    public static final int DNS_TEST = 1;
    public static final int NTP_TEST = 2;
    public static final int TFTP_TEST = 3;
    public static final int FTP_TEST = 4;
    public static final int HTTP_TEST = 5;
    public static final int SIP_TEST = 6;

    public class Test {
        private TableItem tableRow;
        private TestIconUpdater testIconUpdater = null;

        public static final int IDLE = 0;
        public static final int RUNNING = 1;
        public static final int SKIPPED = 2;
        public static final int COMPLETED = 3;

        public Test(String description) {
            tableRow = new TableItem(testSummaryTable, 0);
            tableRow.setImage(0, nullIcon);
            tableRow.setText(1, description);
        }

        public void update(int state) {
            if (!display.isDisposed()) {
                if (state == RUNNING) {
                    if (testIconUpdater == null) {
                        testIconUpdater = new TestIconUpdater(tableRow);
                    }
                } else {
                    if (testIconUpdater != null) {
                        testIconUpdater.quit();
                        testIconUpdater = null;
                    }
                }
                display.asyncExec(new TestTableUpdater(tableRow, state, null));
            }
        }

        public void update(int state, String message) {
            if (!display.isDisposed()) {
                if (state == RUNNING) {
                    if (testIconUpdater == null) {
                        testIconUpdater = new TestIconUpdater(tableRow);
                    }
                } else {
                    if (testIconUpdater != null) {
                        testIconUpdater.quit();
                        testIconUpdater = null;
                    }
                }
                display.asyncExec(new TestTableUpdater(tableRow, state, message));
            }
        }

        public void update(ResultCode results) {
            if (!display.isDisposed()) {
                if (testIconUpdater != null) {
                    testIconUpdater.quit();
                    testIconUpdater = null;
                }
                display.asyncExec(new TestTableUpdater(tableRow, results));
            }
        }

    }

    ShellTestRunner(Display display, TabFolder folder, TabItem testTab, JournalService journalService) {
        this.display = display;
        this.journalService = journalService;

        // Define the font set.
        // normalFont = new Font(null, "Arial", 10, SWT.NORMAL);
        // boldFont = new Font(null, "Arial", 10, SWT.BOLD);

        // Define the color set.
        black = display.getSystemColor(SWT.COLOR_BLACK);
        white = display.getSystemColor(SWT.COLOR_WHITE);
        magenta = display.getSystemColor(SWT.COLOR_MAGENTA);
        red = new Color(null, 237, 24, 30);
        green = new Color(null, 47, 139, 32);
        blue = new Color(null, 0, 0, 192);
        yellow = new Color(null, 255, 210, 60);

        // Create the test status icons.
        nullIcon = new Image(display, 20, 20);
        GC gc = new GC(nullIcon);
        gc.setBackground(display.getSystemColor(SWT.COLOR_WIDGET_LIGHT_SHADOW));
        gc.fillArc(2, 2, 16, 16, 90, 180);
        gc.setForeground(black);
        gc.drawOval(2, 2, 16, 16);
        gc.dispose();

        idleIcon = new Image(display, 20, 20);
        gc = new GC(idleIcon);
        gc.setBackground(white);
        gc.setForeground(black);
        gc.drawOval(2, 2, 16, 16);
        gc.dispose();

        passedIcon = new Image(display, 20, 20);
        gc = new GC(passedIcon);
        gc.setBackground(green);
        gc.fillOval(2, 2, 16, 16);
        gc.dispose();

        failedIcon = new Image(display, 20, 20);
        gc = new GC(failedIcon);
        gc.setBackground(red);
        gc.fillOval(2, 2, 16, 16);
        gc.dispose();

        snafuIcon = new Image(display, 20, 20);
        gc = new GC(snafuIcon);
        gc.setBackground(magenta);
        gc.fillOval(2, 2, 16, 16);
        gc.dispose();

        warningIcon = new Image(display, 20, 20);
        gc = new GC(warningIcon);
        gc.setBackground(yellow);
        gc.fillOval(2, 2, 16, 16);
        gc.dispose();

        skippedIcon = new Image(display, 20, 20);
        gc = new GC(skippedIcon);
        gc.setBackground(blue);
        gc.fillOval(2, 2, 16, 16);
        gc.dispose();

        // Build spinner icon set.
        spinnerIcon = new Image[12];
        for (int x = 0; x < 12; x++) {
        	InputStream iconStream = Shell.class.getClassLoader().getResourceAsStream("icons/rotation" + x + ".png");
            if (iconStream != null) {
            	spinnerIcon[x] = new Image(display, iconStream);
            }
        }

        // Set up the test summary table.
        testSummaryTable = new Table(folder, SWT.SINGLE | SWT.FULL_SELECTION);
        TableColumn col1 = new TableColumn(testSummaryTable, SWT.CENTER);
        col1.setWidth(40);
        TableColumn col2 = new TableColumn(testSummaryTable, SWT.LEFT);
        col2.setWidth(150);
        TableColumn col3 = new TableColumn(testSummaryTable, SWT.CENTER);
        col3.setWidth(70);
        TableColumn col4 = new TableColumn(testSummaryTable, SWT.LEFT);
        col4.setWidth(375);

        testSummaryTable.setHeaderVisible(false);
        testSummaryTable.setLinesVisible(true);

        testTable = new Test[7];

        testTable[DHCP_TEST] = new Test("DHCP Server Test");
        testTable[DNS_TEST] = new Test("DNS Server Test");
        testTable[NTP_TEST] = new Test("NTP Server Test");
        testTable[TFTP_TEST] = new Test("TFTP Server Test");
        testTable[FTP_TEST] = new Test("FTP Server Test");
        testTable[HTTP_TEST] = new Test("HTTP Server Test");
        testTable[SIP_TEST] = new Test("SIP Server Test");

        // table.getColumn(0).pack();
        // table.getColumn(1).pack();

        testTab.setControl(testSummaryTable);

        active = false;
    }

    public void validate() {
        ResultCode results;
        NetworkResources networkResources = new NetworkResources();
        InetAddress bindAddress;
		try {
			bindAddress = InetAddress.getByName("0.0.0.0");
		} catch (UnknownHostException e) {
			e.printStackTrace();
			return;
		}

        active = true;

        testTable[DHCP_TEST].update(Test.IDLE);
        testTable[DNS_TEST].update(Test.IDLE);
        testTable[NTP_TEST].update(Test.IDLE);
        testTable[TFTP_TEST].update(Test.IDLE);
        testTable[FTP_TEST].update(Test.IDLE);
        testTable[HTTP_TEST].update(Test.IDLE);
        testTable[SIP_TEST].update(Test.IDLE);

        DHCP dhcp = new DHCP();
        DNS dns = new DNS();
        NTP ntp = new NTP();
        TFTP tftp = new TFTP();
        FTP ftp = new FTP();
        HTTP http = new HTTP();
        SIPServerTest sipServerTest = new SIPServerTest();

        testTable[DHCP_TEST].update(Test.RUNNING);
        results = dhcp.validate(10, networkResources, journalService, bindAddress);
        testTable[DHCP_TEST].update(results);

        if (results == NONE) {
            testTable[DNS_TEST].update(Test.RUNNING);
            results = dns.validate(10, networkResources, journalService, bindAddress);
            testTable[DNS_TEST].update(results);

            if (networkResources.ntpServers != null) {
                testTable[NTP_TEST].update(Test.RUNNING);
                results = ntp.validate(10, networkResources, journalService, bindAddress);
                testTable[NTP_TEST].update(results);
            } else {
                testTable[NTP_TEST].update(Test.SKIPPED, "No NTP server discovered.");
            }

            if (networkResources.configServer != null) {
                testTable[TFTP_TEST].update(Test.RUNNING);
                results = tftp.validate(10, networkResources, journalService, bindAddress);
                testTable[TFTP_TEST].update(results);

                testTable[FTP_TEST].update(Test.RUNNING);
                results = ftp.validate(10, networkResources, journalService, bindAddress);
                testTable[FTP_TEST].update(results);

                testTable[HTTP_TEST].update(Test.RUNNING);
                results = http.validate(10, networkResources, journalService, bindAddress);
                testTable[HTTP_TEST].update(results);

                testTable[SIP_TEST].update(Test.RUNNING);
                results = sipServerTest.validate(10, networkResources, journalService, bindAddress);
                testTable[SIP_TEST].update(results);
            } else {
                testTable[TFTP_TEST].update(Test.SKIPPED, "No TFTP server discovered.");
                testTable[FTP_TEST].update(Test.SKIPPED, "No FTP server discovered.");
                testTable[HTTP_TEST].update(Test.SKIPPED, "No HTTP server discovered.");
                testTable[SIP_TEST].update(Test.SKIPPED, "No SIP server discovered.");
            }
        } else {
            testTable[DNS_TEST].update(Test.SKIPPED, "DHCP prerequisite test failed.");
            testTable[NTP_TEST].update(Test.SKIPPED, "DHCP prerequisite test failed.");
            testTable[TFTP_TEST].update(Test.SKIPPED, "DHCP prerequisite test failed.");
            testTable[FTP_TEST].update(Test.SKIPPED, "DHCP prerequisite test failed.");
            testTable[HTTP_TEST].update(Test.SKIPPED, "DHCP prerequisite test failed.");
            testTable[SIP_TEST].update(Test.SKIPPED, "SIP prerequisite test failed.");
        }

        active = false;
    }

    public boolean isActive() {
        return active;
    }

    private class TestTableUpdater implements Runnable {
        private TableItem test;
        private String message;
        private ResultCode results;
        private int state;

        TestTableUpdater(TableItem test, int state, String message) {
            this.test = test;
            this.results = null;
            this.state = state;
            this.message = message;
        }

        TestTableUpdater(TableItem test, ResultCode results) {
            this.test = test;
            this.results = results;
            state = Test.COMPLETED;
            message = null;
        }

        public void run() {
            if (!test.isDisposed()) {
                switch (state) {
                    case Test.IDLE:
                        test.setImage(0, idleIcon);
                        test.setText(2, "");
                        if (message == null) {
                            test.setText(3, "");
                        } else {
                            test.setText(3, message);
                        }
                        break;
                    case Test.RUNNING:
                        test.setImage(0, idleIcon);
                        test.setForeground(2, black);
                        test.setText(2, "Executing");
                        if (message == null) {
                            test.setText(3, "");
                        } else {
                            test.setText(3, message);
                        }
                        break;
                    case Test.SKIPPED:
                        test.setImage(0, skippedIcon);
                        test.setForeground(2, blue);
                        test.setText(2, "Skipped");
                        if (message == null) {
                            test.setText(3, "");
                        } else {
                            test.setText(3, message);
                        }
                        break;
                    case Test.COMPLETED:
                        if (results == NONE) {
                            test.setImage(0, passedIcon);
                            test.setForeground(2, green);
                            test.setText(2, "Passed");
                        } else {
                            switch (results.toResultClass()) {
                                case UUT:
                                    test.setImage(0, failedIcon);
                                    test.setForeground(2, red);
                                    test.setText(2, "Failed");
                                    break;
                                case TOOL:
                                    test.setImage(0, snafuIcon);
                                    test.setForeground(2, magenta);
                                    test.setText(2, "ERROR");
                                    break;
                                case WARNING:
                                    test.setImage(0, warningIcon);
                                    test.setForeground(2, yellow);
                                    test.setText(2, "Warning");
                                    break;
                            }
                            test.setText(3, results.toString());
                        }
                }
            }
        }
    }

    private class TestIconUpdater extends Thread {
        private TableItem test;
        private int sequence;
        private volatile boolean active;

        public TestIconUpdater(TableItem test) {
            this.test = test;
            sequence = 0;
            active = true;
            start();
        }

        public void run() {
            while (active) {
                if (display.isDisposed()) {
                    active = false;
                    return;
                }
                display.asyncExec(new Runnable() {
                    public void run() {
                        if (!test.isDisposed()) {
                            test.setImage(0, spinnerIcon[sequence]);
                            if (++sequence == 12) {
                                sequence = 0;
                            }
                        }
                    }
                });
                try {
                    super.sleep(100);
                } catch (Throwable th) {
                }
                if (display.isDisposed()) {
                    active = false;
                    return;
                }
            }
        }

        public void quit() {
            active = false;
        }
    }

}
