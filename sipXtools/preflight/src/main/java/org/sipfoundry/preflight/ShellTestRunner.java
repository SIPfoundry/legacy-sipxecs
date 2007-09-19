/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import org.eclipse.swt.*;
import org.eclipse.swt.graphics.*;
import org.eclipse.swt.widgets.*;

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

    //private final Font normalFont;
    //private final Font boldFont;
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
    private final TableItem[] testTable;
    private boolean active;

    public static final int DHCP_TEST = 0;
    public static final int DNS_TEST  = 1;
    public static final int NTP_TEST  = 2;
    public static final int TFTP_TEST = 3;
    public static final int FTP_TEST  = 4;
    public static final int HTTP_TEST = 5;
    public static final int SIP1_TEST = 6;
    public static final int SIP2_TEST = 7;
    
    ShellTestRunner(Display display, TabFolder folder, TabItem testTab, JournalService journalService) {
        this.display = display;
        this.journalService = journalService;

        // Define the font set.
        //normalFont = new Font(null, "Arial", 10, SWT.NORMAL);
        //boldFont = new Font(null, "Arial", 10, SWT.BOLD);
        
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
        gc.fillOval(2, 2, 16, 16);
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
        spinnerIcon = new Image[5];
        int angle = 68;
        for (int x = 0; x < 5; x++) {
            spinnerIcon[x] = new Image(display, 20, 20);
            gc = new GC(spinnerIcon[x]);
            gc.setBackground(black);
        	gc.fillArc(2, 2, 16, 16, angle, 45);
			gc.fillArc(2, 2, 16, 16, 90 + angle, 45);
			gc.fillArc(2, 2, 16, 16, 180 + angle, 45);
			gc.fillArc(2, 2, 16, 16, 270 + angle, 45);
			gc.dispose();
			angle -= 18;
        }
        

        // Set up the test summary table.
        final Table table = new Table(folder, SWT.SINGLE | SWT.FULL_SELECTION);
        TableColumn col1 = new TableColumn(table, SWT.CENTER);
        col1.setWidth(40);
        TableColumn col2 = new TableColumn(table, SWT.LEFT);
        col2.setWidth(150);
        TableColumn col3 = new TableColumn(table, SWT.CENTER);
        col3.setWidth(60);
        TableColumn col4 = new TableColumn(table, SWT.LEFT);
        col4.setWidth(365);

        testTable = new TableItem[8];

        testTable[DHCP_TEST] = new TableItem(table, 0);
        testTable[DHCP_TEST].setImage(0, spinnerIcon[3]);
        testTable[DHCP_TEST].setText(1, "DHCP Server Test");

        testTable[DNS_TEST] = new TableItem(table, 0);
        testTable[DNS_TEST].setImage(0, spinnerIcon[3]);
        testTable[DNS_TEST].setText(1, "DNS Server Test");

        testTable[NTP_TEST] = new TableItem(table, 0);
        testTable[NTP_TEST].setImage(0, spinnerIcon[3]);
        testTable[NTP_TEST].setText(1, "NTP Server Test");

        testTable[TFTP_TEST] = new TableItem(table, 0);
        testTable[TFTP_TEST].setImage(0, spinnerIcon[3]);
        testTable[TFTP_TEST].setText(1, "TFTP Server Test");

        testTable[FTP_TEST] = new TableItem(table, 0);
        testTable[FTP_TEST].setImage(0, spinnerIcon[3]);
        testTable[FTP_TEST].setText(1, "FTP Server Test");

        testTable[HTTP_TEST] = new TableItem(table, 0);
        testTable[HTTP_TEST].setImage(0, spinnerIcon[3]);
        testTable[HTTP_TEST].setText(1, "HTTP Server Test");

        testTable[SIP1_TEST] = new TableItem(table, 0);
        testTable[SIP1_TEST].setImage(0, spinnerIcon[3]);
        testTable[SIP1_TEST].setText(1, "SIP Connectivity Test");

        testTable[SIP2_TEST] = new TableItem(table, 0);
        testTable[SIP2_TEST].setImage(0, spinnerIcon[3]);
        testTable[SIP2_TEST].setText(1, "SIP Call Control Test");

        table.setHeaderVisible(false);
        table.setLinesVisible(true);

        // table.getColumn(0).pack();
        // table.getColumn(1).pack();

        testTab.setControl(table);

        active = false;
    }

    public void validate() {
        TestIconUpdater testIconUpdater;
        ResultCode results;
        NetworkResources networkResources = new NetworkResources();

        active = true;
        
        update(DHCP_TEST, TestTableUpdater.IDLE);
        update(DNS_TEST, TestTableUpdater.IDLE);
        update(NTP_TEST, TestTableUpdater.IDLE);
        update(TFTP_TEST, TestTableUpdater.IDLE);
        update(FTP_TEST, TestTableUpdater.IDLE);
        update(HTTP_TEST, TestTableUpdater.IDLE);
        update(SIP1_TEST, TestTableUpdater.IDLE);
        update(SIP2_TEST, TestTableUpdater.IDLE);
        
        DHCP dhcp = new DHCP();
        DNS dns = new DNS();
        NTP ntp = new NTP();
        TFTP tftp = new TFTP();

        
        update(DHCP_TEST, TestTableUpdater.RUNNING);
        testIconUpdater = new TestIconUpdater(testTable[DHCP_TEST]);
        results = dhcp.validate(20, networkResources, journalService);
        testIconUpdater.quit();
        update(DHCP_TEST, results);

        if (results == NONE) {
        	update(DNS_TEST, TestTableUpdater.RUNNING);
        	testIconUpdater = new TestIconUpdater(testTable[DNS_TEST]);
        	results = dns.validate(20, networkResources, journalService);
        	testIconUpdater.quit();
        	update(DNS_TEST, results);
        
            if (networkResources.ntpServers != null) {
                update(NTP_TEST, TestTableUpdater.RUNNING);
        		testIconUpdater = new TestIconUpdater(testTable[NTP_TEST]);
        		results = ntp.validate(20, networkResources, journalService);
        		testIconUpdater.quit();
        		update(NTP_TEST, results);
            } else {
                update(NTP_TEST, TestTableUpdater.SKIPPED, "No NTP server discovered.");
            }
            
            if (networkResources.tftpServer != null) {
                update(TFTP_TEST, TestTableUpdater.RUNNING);
        		testIconUpdater = new TestIconUpdater(testTable[TFTP_TEST]);
        		results = tftp.validate(20, networkResources, journalService);
        		testIconUpdater.quit();
        		update(TFTP_TEST, results);
            } else {
                update(TFTP_TEST, TestTableUpdater.SKIPPED, "No TFTP server discovered.");
            }
        } else {
            update(DNS_TEST, TestTableUpdater.SKIPPED, "DHCP prerequisite test failed.");
        	update(NTP_TEST, TestTableUpdater.SKIPPED, "DHCP prerequisite test failed.");
        	update(TFTP_TEST, TestTableUpdater.SKIPPED, "DHCP prerequisite test failed.");
        }
        
        update(FTP_TEST, TestTableUpdater.SKIPPED, "No FTP server address configured.");
        update(HTTP_TEST, TestTableUpdater.SKIPPED, "No HTTP server address configured.");
        update(SIP1_TEST, TestTableUpdater.SKIPPED);
        update(SIP2_TEST, TestTableUpdater.SKIPPED);
        
        active = false;
    }

    private void update(int test, int state) {
        if (!display.isDisposed()) {
            display.asyncExec(new TestTableUpdater(testTable[test], state, null));
        }
    }
    
    private void update(int test, int state, String message) {
        if (!display.isDisposed()) {
            display.asyncExec(new TestTableUpdater(testTable[test], state, message));
        }
    }
    
    private void update(int test, ResultCode results) {
        if (!display.isDisposed()) {
            display.asyncExec(new TestTableUpdater(testTable[test], results));
        }
    }
    
    public boolean isActive() {
        return active;
    }
    
    private class TestTableUpdater implements Runnable {
        private TableItem test;
        private String message;
        private ResultCode results;
        private int state;
        
        public static final int IDLE = 0;
        public static final int RUNNING = 1;
        public static final int SKIPPED = 2;
        public static final int COMPLETED = 3;

        TestTableUpdater(TableItem test, int state, String message) {
            this.test = test;
            this.results = null;
            this.state = state;
            this.message = message;
        }

        TestTableUpdater(TableItem test, ResultCode results) {
            this.test = test;
            this.results = results;
            state = COMPLETED;
            message = null;
        }

        public void run() {
            if (!test.isDisposed()) {
                switch (state) {
                    case IDLE:
                        test.setImage(0, idleIcon);
                        test.setText(2, "");
                        if (message == null) {
                            test.setText(3, "");
                        } else {
                            test.setText(3, message);
                        }
                        break;
                    case RUNNING:
                        test.setImage(0, idleIcon);
                        test.setForeground(2, black);
                        test.setText(2, "Executing");
                        if (message == null) {
                            test.setText(3, "");
                        } else {
                            test.setText(3, message);
                        }
                        break;
                    case SKIPPED:
                        test.setImage(0, skippedIcon);
                        test.setForeground(2, blue);
                        test.setText(2, "Skipped");
                        if (message == null) {
                            test.setText(3, "");
                        } else {
                            test.setText(3, message);
                        }
                        break;
                    case COMPLETED:
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
                                    test.setText(2, "FUBAR");
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
                            if (++sequence == 5) {
                                sequence = 0;
                            }
                        }
                    }
                });
                try {
                    super.sleep(25);
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
