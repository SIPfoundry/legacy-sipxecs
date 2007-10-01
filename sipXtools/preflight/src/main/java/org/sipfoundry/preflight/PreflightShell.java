/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
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
    private TabItem discoveryTab = null;
    private ShellJournalService shellJournalService;
    private ShellJournalService discoveryJournalService;
    private ShellTestRunner shellTestRunner;
    private ProgressBar discoveryProgressBar;
    private int discoveryProgressBarMax;

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
        runButton.setText("Network Test");
        shell.setDefaultButton(runButton);

        final Button sipButton = new Button(shell, SWT.PUSH);
        formData = new FormData();
        formData.left = new FormAttachment(runButton, 5);
        formData.bottom = new FormAttachment(100, -5);
        sipButton.setLayoutData(formData);
        sipButton.setText("SIP Test");
        sipButton.setEnabled(false);

        final Button discoveryButton = new Button(shell, SWT.PUSH);
        formData = new FormData();
        formData.left = new FormAttachment(sipButton, 5);
        formData.bottom = new FormAttachment(100, -5);
        discoveryButton.setLayoutData(formData);
        discoveryButton.setText("Device Discovery");
        discoveryButton.setEnabled(true);

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
        
        discoveryButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                if (discoveryTab == null) {
                	discoveryTab = new TabItem(folder, SWT.NONE);
        		    discoveryTab.setText("Device Discovery");
        		    
        		    Composite discoveryComposite = new Composite(folder, SWT.NONE);
        		    discoveryComposite.setLayout(new GridLayout());
        		    
        		    List list = new List(discoveryComposite, SWT.BORDER | SWT.V_SCROLL);
        			list.setLayoutData(new GridData(GridData.FILL_BOTH | GridData.GRAB_HORIZONTAL | GridData.GRAB_VERTICAL));
        			discoveryJournalService = new ShellJournalService(display, list);
        			
        	        discoveryProgressBar = new ProgressBar(discoveryComposite, SWT.SMOOTH);
        	        discoveryProgressBar.setBounds(10, 10, 300, 32);
        	        discoveryProgressBar.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        	        discoveryProgressBarMax = discoveryProgressBar.getMaximum();
        	        discoveryProgressBar.setSelection(discoveryProgressBarMax);
        	        
        			discoveryTab.setControl(discoveryComposite);

                }
        		folder.setSelection(discoveryTab);
        		shell.setDefaultButton(discoveryButton);
        		
        		// Test the progress bar.
        		if (discoveryProgressBar.getSelection() == discoveryProgressBarMax) {
        		    discoveryJournalService.println("Starting device discovery...");
        		    new Thread() {
        		        public void run() {
        		            for (final int[] i = new int[1]; i[0] <= discoveryProgressBarMax; i[0]++) {
        		                try {
        		                    Thread.sleep(200);
        		                } catch (Throwable th) {
        		                }
        		                if (display.isDisposed())
        		                    return;
        		                display.asyncExec(new Runnable() {
        		                    public void run() {
        		                        if (!discoveryProgressBar.isDisposed()) {
        		                            discoveryProgressBar.setSelection(i[0]);
        		                        }
        		                    }
        		                });
        		            }
        		            discoveryJournalService.println("Discovery timeout.\n");
        		        }
        		    }.start();
        		}

            }
        });
        
        quitButton.addSelectionListener(new SelectionAdapter() {
            public void widgetSelected(SelectionEvent e) {
                shell.close();
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
        PreflightShell userInterface = new PreflightShell();
        userInterface.go(args);
    }
}
