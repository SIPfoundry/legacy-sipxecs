/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.moh;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.imdb.ReplicationManager;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;

public class MusicOnHoldManagerImpl implements MusicOnHoldManager, ReplicableProvider, DaoEventListener,
        BeanFactoryAware, FeatureProvider {
    public static final Log LOG = LogFactory.getLog(MusicOnHoldManagerImpl.class);
    private String m_audioDirectory;
    private String m_mohUser;
    private ListableBeanFactory m_beanFactory;
    private FeatureManager m_featureManager;
    private BeanWithSettingsDao<MohSettings> m_settingsDao;
    private ReplicationManager m_replicationManager;

    /**
     * Music on hold implementation requires that ~~mh~u calls are forwarded to Media Server. We
     * are adding the rule here.
     */
    public List<DialingRule> getDialingRules(Location location) {
        MohAddressFactory factory = getAddressFactory();
        Address mediaAddress = factory.getMediaAddress(location);
        if (mediaAddress == null) {
            return Collections.emptyList();
        }
        DialingRule rule = new MohRule(mediaAddress.getAddress(), factory.getFilesMohUser());
        return Collections.singletonList(rule);
    }

    public String getAudioDirectoryPath() {
        ensureDirectoryExists(m_audioDirectory);
        return m_audioDirectory;
    }

    public File getUserAudioDirectory(User user) {
        File userAudioDirectory = new File(m_audioDirectory + System.getProperty("file.separator") + user.getName());
        ensureDirectoryExists(userAudioDirectory.getPath());
        return userAudioDirectory;
    }

    public boolean isAudioDirectoryEmpty() {
        File audioDirectory = new File(m_audioDirectory);
        File[] mohFiles = audioDirectory.listFiles();
        return mohFiles == null ? true : mohFiles.length == 0;
    }

    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        if (alias.startsWith(m_mohUser)) {
            // id = 1 because there's only 1 MOH instance
            return BeanId.createBeanIdCollection(Collections.singleton(1), this.getClass());
        }
        return Collections.emptyList();
    }

    public boolean isAliasInUse(String alias) {
        if (alias.startsWith(m_mohUser)) {
            return true;
        }
        return false;
    }

    public void onDelete(Object entity) {
        if (entity instanceof User) {
            File userAudioDirectory = getUserAudioDirectory((User) entity);
            if (userAudioDirectory.exists()) {
                try {
                    FileUtils.deleteDirectory(userAudioDirectory);
                } catch (IOException e) {
                    LOG.error("Failed to delete user Music on Hold directory", e);
                }
            }
        }
    }

    public void onSave(Object entity) {
    }

    @Required
    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    @Required
    public void setMohUser(String mohUser) {
        m_mohUser = mohUser;
    }

    public MohAddressFactory getAddressFactory() {
        return m_beanFactory.getBean(MohAddressFactory.class);
    }

    @Override
    public List<Replicable> getReplicables() {
        List<Location> locations = m_featureManager.getLocationsForEnabledFeature(FEATURE);
        if (locations.isEmpty()) {
            return Collections.emptyList();
        }
        List<Replicable> replicables = new ArrayList<Replicable>();
        replicables.add(getSettings());
        return replicables;
    }

    /**
     * If the directory does not exist create it.
     *
     * HACK: sipXconfig should not create those directories, services that owns directories should
     * be responsible for that
     *
     * @param path
     */
    private void ensureDirectoryExists(String path) {
        try {
            File file = new File(path);
            FileUtils.forceMkdir(file);
        } catch (IOException e) {
            LOG.error("Cannot create directory: " + path);
            throw new RuntimeException(e);
        }
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public MohSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(MohSettings settings) {
        m_settingsDao.upsert(settings);
        m_replicationManager.replicateEntity(settings);
    }

    public void setSettingsDao(BeanWithSettingsDao<MohSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public Collection<GlobalFeature> getAvailableGlobalFeatures(FeatureManager featureManager) {
        return null;
    }

    @Override
    public Collection<LocationFeature> getAvailableLocationFeatures(FeatureManager featureManager, Location l) {
        return Collections.singleton(FEATURE);
    }

    @Override
    public void getBundleFeatures(FeatureManager featureManager, Bundle b) {
        if (b == Bundle.CORE_TELEPHONY) {
            b.addFeature(FEATURE);
        }
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(FEATURE, FreeswitchFeature.FEATURE);
        validator.singleLocationOnly(FEATURE);
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
    }
}
