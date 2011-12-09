/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin;

import static org.springframework.dao.support.DataAccessUtils.singleResult;

import java.io.File;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.backup.FtpBackupPlan;
import org.sipfoundry.sipxconfig.backup.LocalBackupPlan;
import org.sipfoundry.sipxconfig.bulk.ExportCsv;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.sipfoundry.sipxconfig.common.DSTChangeEvent;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.ftp.FtpConfiguration;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

/**
 * Backup provides Java interface to backup scripts
 */
public abstract class AdminContextImpl extends HibernateDaoSupport implements AdminContext, ApplicationListener {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(HTTP_ADDRESS, HTTPS_ADDRESS,
            TFTP_ADDRESS, FTP_ADDRESS);
    private String m_binDirectory;
    private String m_libExecDirectory;
    private ExportCsv m_exportCsv;

    public abstract FtpBackupPlan createFtpBackupPlan();

    public abstract LocalBackupPlan createLocalBackupPlan();

    @Override
    public Collection<AddressType> getSupportedAddressTypes(AddressManager manager) {
        return ADDRESSES;
    }

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Object requester) {
        if (ADDRESSES.contains(type)) {
            List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(FEATURE);
            List<Address> addresses = new ArrayList<Address>(locations.size());
            for (Location location : locations) {
                Address address = new Address();
                address.setAddress(location.getAddress());
                if (type.equals(HTTP_ADDRESS)) {
                    address.setPort(12000);
                } else if (type.equals(HTTPS_ADDRESS)) {
                    address.setPort(8443);
                }
            }
            return addresses;

        }
        return null;
    }

    public String getBinDirectory() {
        return m_binDirectory;
    }

    public void setBinDirectory(String binDirectory) {
        m_binDirectory = binDirectory;
    }

    public String getLibExecDirectory() {
        return m_libExecDirectory;
    }

    public void setLibExecDirectory(String libExecDirectory) {
        m_libExecDirectory = libExecDirectory;
    }

    public void setExportCsv(ExportCsv exportCsv) {
        m_exportCsv = exportCsv;
    }

    public BackupPlan getBackupPlan(String type) {
        boolean isFtp = FtpBackupPlan.TYPE.equals(type);
        Class klass = isFtp ? FtpBackupPlan.class : LocalBackupPlan.class;
        List plans = getHibernateTemplate().loadAll(klass);
        BackupPlan plan = (BackupPlan) DataAccessUtils.singleResult(plans);
        if (plan == null) {
            plan = isFtp ? createFtpBackupPlan() : createLocalBackupPlan();
            if (isFtp) {
                initFtpConfig(plan);
            }
            getHibernateTemplate().save(plan);
            getHibernateTemplate().flush();
        }
        return plan;
    }

    private void initFtpConfig(BackupPlan plan) {
        FtpBackupPlan ftpBackupPlan = (FtpBackupPlan) plan;
        if (ftpBackupPlan.getFtpConfiguration() != null) {
            return;
        }
        List ftpConfigs = getHibernateTemplate().loadAll(FtpConfiguration.class);
        FtpConfiguration ftpConfig = (FtpConfiguration) singleResult(ftpConfigs);
        if (ftpConfig == null) {
            ftpConfig = new FtpConfiguration();
        }
        ftpBackupPlan.setFtpConfiguration(ftpConfig);
    }

    public void storeBackupPlan(BackupPlan plan) {
        getHibernateTemplate().saveOrUpdate(plan);
        plan.resetTimer(m_binDirectory);
    }

    public File[] performBackup(BackupPlan plan) {
        return plan.perform(m_binDirectory);
    }

    public void performExport(Writer writer) throws IOException {
        m_exportCsv.exportCsv(writer);
    }

    /**
     * start backup timers after app is initialized
     */
    public void onApplicationEvent(ApplicationEvent event) {
        // No need to register listener, all beans that implement listener
        // interface are
        // automatically registered
        if (event instanceof ApplicationInitializedEvent || event instanceof DSTChangeEvent) {
            List<BackupPlan> plans = getHibernateTemplate().loadAll(BackupPlan.class);
            for (BackupPlan plan : plans) {
                plan.resetTimer(m_binDirectory);
            }
        }
    }

    public String[] getInitializationTasks() {
        List l = getHibernateTemplate().findByNamedQuery("taskNames");
        return (String[]) l.toArray(new String[l.size()]);
    }

    public void deleteInitializationTask(String task) {
        List l = getHibernateTemplate().findByNamedQueryAndNamedParam("taskByName", "task", task);
        getHibernateTemplate().deleteAll(l);
    }

    public boolean inInitializationPhase() {
        String initializationPhase = System.getProperty("sipxconfig.initializationPhase");
        if (initializationPhase == null) {
            return false;
        }

        return Boolean.parseBoolean(initializationPhase);
    }
}
