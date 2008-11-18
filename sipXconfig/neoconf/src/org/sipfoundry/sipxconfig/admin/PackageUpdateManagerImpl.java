/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.apache.commons.io.IOUtils;

public class PackageUpdateManagerImpl implements Serializable, PackageUpdateManager {

    private static final String PACKAGE_BINARY = "sipxpackage";

    /** Executor that runs the package update tasks in the background. */
    private ExecutorService m_updateExecutor = Executors.newSingleThreadExecutor();
    
    /** The current state of the updater. */
    private UpdaterState m_state = UpdaterState.NO_UPDATES_AVAILABLE;
    
    /** The current list of available sipXecs package updates. */
    private List<PackageUpdate> m_availablePackages = new ArrayList<PackageUpdate>();
    
    /** The last time we checked for updates. */
    private Date m_lastCheckedDate;
    
    private AdminContext m_adminContext;

    /** The currently installed sipx version. */
    private String m_currentVersion;
    
    /** The available updated sipx version. */
    private String m_updatedVersion;
    
    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }

    public List<PackageUpdate> getAvailablePackages() {
        return m_availablePackages;
    }
    
    public Date getLastCheckedDate() {
        return m_lastCheckedDate;
    }
    
    public UpdaterState getState() {
        return m_state;
    }
    
    private Process runPackageCommand(String argument) throws IOException {
        String binaryPath = m_adminContext.getLibExecDirectory() + File.separator + PACKAGE_BINARY;
        ProcessBuilder packageProcessBuilder = new ProcessBuilder(binaryPath, argument);       
        packageProcessBuilder.redirectErrorStream(true);
        
        return packageProcessBuilder.start();
    }    
    
    public void checkForUpdates() {
        m_state = UpdaterState.CHECKING_FOR_UPDATES;
        m_updateExecutor.submit(new CheckForUpdatesTask());
    }
    
//    public void installUpdates() {
//        m_state = UpdaterState.INSTALLING;
//        m_updateExecutor.submit(new InstallUpdatesTask());
//    }    
    
    public void restart() {
        try {
            Process packageProcess = runPackageCommand("restart");
            packageProcess.waitFor();
        } catch (IOException ioe) {
            ioe.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
    
    private void updateCurrentVersion() {
        BufferedReader reader = null;
        try {
            Process packageProcess = runPackageCommand("version");
            reader = new BufferedReader(new InputStreamReader(packageProcess.getInputStream()));
            
            String versionLine = reader.readLine();
            m_currentVersion = versionLine;
        } catch (IOException ioe) {
            ioe.printStackTrace();
        } finally {
            IOUtils.closeQuietly(reader);
        }
    }
    
    public String getCurrentVersion() {
        if (m_currentVersion == null) {
            updateCurrentVersion();
        }
        
        return m_currentVersion;
    }
    
    public String getUpdatedVersion() {
        return m_updatedVersion;
    }

    public String getUpdateBinaryPath() {
        return m_adminContext.getLibExecDirectory() + File.separator + PACKAGE_BINARY;
    }
    
    /**
     * This task checks for updated sipxecs packages and updates the UI accordingly.
     */
    class CheckForUpdatesTask implements Runnable {
        
        public void run() {
            m_lastCheckedDate = new Date();
            List<PackageUpdate> updates = new ArrayList<PackageUpdate>();
            
            BufferedReader packageReader = null;
            try {
                Process packageProcess = runPackageCommand("check-update");
                packageReader = new BufferedReader(
                        new InputStreamReader(packageProcess.getInputStream()));

                String line;
                String[] packageInfo;
                while ((line = packageReader.readLine()) != null) {
                    if (!line.startsWith("#")) {
                        packageInfo = line.split("\\|");
                        if (packageInfo.length == 3) {
                            if (packageInfo[0].equals("sipxecs")) {
                                m_updatedVersion = "sipxecs " + packageInfo[2];
                            }
                            updates.add(new PackageUpdate(packageInfo[0], packageInfo[1], packageInfo[2]));
                        }
                    }
                }
                
                int returnValue = packageProcess.waitFor();
            } catch (IOException e) {
                e.printStackTrace();
            } catch (InterruptedException e) {
                e.printStackTrace();
            } finally {
                IOUtils.closeQuietly(packageReader);
            }
            
            m_availablePackages = updates;
            m_state = (updates.isEmpty()) ? UpdaterState.NO_UPDATES_AVAILABLE : UpdaterState.UPDATES_AVAILABLE;
        }
    }

//    class InstallUpdatesTask implements Runnable {
//        public void run() {
//            
//            int returnValue = -1;
//            BufferedReader installReader = null;
//            try {
//                Process installprocess = runPackageCommand("update");
//                installReader = new BufferedReader(new InputStreamReader(installprocess.getInputStream()));
//                
//                String line;
//                while ((line = installReader.readLine()) != null) {
//                }
//                
//                returnValue = installprocess.waitFor();
//            } catch (IOException ioe) {
//                ioe.printStackTrace();
//            } catch (InterruptedException ie) {
//                ie.printStackTrace();
//            } finally {
//                IOUtils.closeQuietly(installReader);
//            }
//            
//            if (returnValue == 0) { // successful update
//                m_updatedVersion = null;
//                m_availablePackages = null;
//                m_state = UpdaterState.UPDATE_COMPLETED;
//            }
//        }
//    }
}
