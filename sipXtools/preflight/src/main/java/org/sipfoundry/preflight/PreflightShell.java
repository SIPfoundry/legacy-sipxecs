/**
 * Copyright (C) 2007 - 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.preflight;

import java.io.*;

import org.eclipse.swt.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.graphics.*;

import org.sipfoundry.commons.util.JournalService;

/**
 * [Enter descriptive text here]
 * <p>
 *
 * @author Mardy Marshall
 */
public class PreflightShell {
    private Display display;
    private TabFolder folder;
    private TabItem testTab = null;
    private TabItem journalTab = null;
    private ShellJournalService shellJournalService;
    private ShellTestRunner shellTestRunner;

    public void go(String[] args) {
        FormData formData;

        display = new Display();
        final Shell shell = new Shell(display);
        shell.setText("Preflight");
        InputStream iconStream = Shell.class.getClassLoader().getResourceAsStream("icons/preflight.png");
        if (iconStream != null) {
            shell.setImage(new Image(display, iconStream));
        }
        shell.setBounds(20, 20, 670, 400);
        shell.setMinimumSize(670, 400);

        shell.setLayoutData(new GridData(GridData.FILL_BOTH | GridData.GRAB_HORIZONTAL | GridData.GRAB_VERTICAL));
        shell.setLayout(new FormLayout());

        // Set up the Configuration menu.
        Menu menuBar = new Menu(shell,SWT.BAR);
        shell.setMenuBar(menuBar);
        MenuItem configurationItem = new MenuItem(menuBar,SWT.CASCADE);
        configurationItem.setText("Configuration");
        Menu configurationMenu = new Menu(shell,SWT.DROP_DOWN);
        configurationItem.setMenu(configurationMenu);
        MenuItem netSettingsItem = new MenuItem(configurationMenu,SWT.PUSH);
        netSettingsItem.setText("Network Test Settings");

        // Set up the buttons.
        final Button runButton = new Button(shell, SWT.PUSH);
        formData = new FormData();
        formData.left = new FormAttachment(0, 5);
        formData.bottom = new FormAttachment(100, -5);
        runButton.setLayoutData(formData);
        runButton.setText("Network Tests");
        shell.setDefaultButton(runButton);

        /*
        final Button sipButton = new Button(shell, SWT.PUSH);
        formData = new FormData();
        formData.left = new FormAttachment(runButton, 5);
        formData.bottom = new FormAttachment(100, -5);
        sipButton.setLayoutData(formData);
        sipButton.setText("SIP Tests");
        sipButton.setEnabled(false);
        */

        final Button quitButton = new Button(shell, SWT.PUSH);
        formData = new FormData();
        formData.right = new FormAttachment(100, -5);
        formData.bottom = new FormAttachment(100, -5);
        quitButton.setLayoutData(formData);
        quitButton.setText("Quit");

        // Set up the tabed Test Summary and Test Journal views.
        folder = new TabFolder(shell,SWT.NONE);
        formData = new FormData();
        formData.top = new FormAttachment(0, 5);
        formData.bottom = new FormAttachment(quitButton, -5);
        formData.right = new FormAttachment(100, -5);
        formData.left = new FormAttachment(0, 5);
        folder.setLayoutData(formData);
        testTab = new TabItem(folder,SWT.NONE);
        testTab.setText("Test Summary");
        journalTab = new TabItem(folder,SWT.NONE);
        journalTab.setText("Test Journal");

        // Set up the Test Journal list view and journal service.
        List list = new List(folder, SWT.BORDER | SWT.V_SCROLL);
        list.setLayoutData(new GridData(GridData.FILL_VERTICAL | GridData.GRAB_HORIZONTAL | GridData.GRAB_VERTICAL));
        shellJournalService = new ShellJournalService(display, list);
        journalTab.setControl(list);

        // Set up the Test Summary test runner.
        shellTestRunner = new ShellTestRunner(display, folder, testTab, shellJournalService);

        runButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                if (!shellTestRunner.isActive()) {
                    TabItem[] selections = folder.getSelection();
                    if (selections[0] != testTab && selections[0] != journalTab) {
                        folder.setSelection(testTab);
                    }
                    shell.setDefaultButton(runButton);
                    shellJournalService.println("Starting network tests.\n");

                    new Thread() {
                        public void run() {
                            shellTestRunner.validate();
                        }
                    }.start();
                }
            }
        });

        quitButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                shell.close();
                System.exit(0);
            }
        });

        shell.open();

        while (!shell.isDisposed()) {
            if (!display.readAndDispatch()) {
                display.sleep();
            }
        }
        display.dispose();
    }

    public static void main(String[] args) {
        class ConsoleJournalService implements JournalService {
            private boolean isEnabled = true;

            public void enable() {
                isEnabled = true;
            }

            public void disable() {
                isEnabled = false;
            }

            public void print(String message) {
                if (isEnabled) {
                    System.out.print(message);
                }
            }

            public void println(String message) {
                if (isEnabled) {
                    System.out.println(message);
                }
            }
        }

        if (System.getProperty("os.name").toLowerCase().contains("windows")) {
            String path = System.getProperty("ArpTable.dll.path", "");
            System.loadLibrary(path + "ArpTable");
            PreflightShell userInterface = new PreflightShell();
        	userInterface.go(args);
        } else {
            ConsoleTestRunner userInterface = new ConsoleTestRunner(new ConsoleJournalService());
            userInterface.validate(args);
        }
    }
}
