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

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.service.ServiceConfigurator;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService.SystemMohSetting;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.freeswitch.LocalStreamConfiguration;

import org.springframework.beans.factory.annotation.Required;

public class MusicOnHoldManagerImpl implements MusicOnHoldManager, DaoEventListener {

    public static final Log LOG = LogFactory.getLog(MusicOnHoldManagerImpl.class);

    private SipxServiceManager m_sipxServiceManager;
    private String m_audioDirectory;
    private SipxReplicationContext m_replicationContext;
    private ServiceConfigurator m_serviceConfigurator;

    private String m_mohUser;
    private String m_localFilesMohUser;
    private String m_portAudioMohUser;
    private String m_personalMohFilesPrefix;

    /**
     * Music on hold implementation requires that ~~mh~ calls are forwarded to Media Server. We
     * are adding the rule here.
     */
    public List<DialingRule> getDialingRules() {
        SipxFreeswitchService service = getSipxFreeswitchService();
        String hostNameAndPort = service.getAddress() + ":" + String.valueOf(service.getFreeswitchSipPort());
        DialingRule rule = new MohRule(hostNameAndPort, m_mohUser);
        return Collections.singletonList(rule);
    }

    public void replicateMohConfiguration() {
        SipxFreeswitchService service = getSipxFreeswitchService();
        List< ? extends ConfigurationFile> configurationFiles = service.getConfigurations();

        for (ConfigurationFile configurationFile : configurationFiles) {
            if (configurationFile instanceof LocalStreamConfiguration) {
                m_replicationContext.replicate(configurationFile);
                m_serviceConfigurator.markServiceForRestart(service);
            }
        }
    }

    public void replicateAliasData() {
        m_replicationContext.generate(DataSet.ALIAS);
    }

    public String getAudioDirectoryPath() {
        ensureDirectoryExists(m_audioDirectory);
        return m_audioDirectory;
    }

    public File getUserAudioDirectory(User user) {
        File userAudioDirectory = new File(m_audioDirectory + user.getName());
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
            List<Integer> ids = new ArrayList<Integer>(1);
            ids.add(getSipxFreeswitchService().getId());
            Collection bids = BeanId.createBeanIdCollection(ids, SipxFreeswitchService.class);
            return bids;
        }
        return CollectionUtils.EMPTY_COLLECTION;
    }

    public boolean isAliasInUse(String alias) {
        if (alias.startsWith(m_mohUser)) {
            return true;
        }
        return false;
    }

    public Collection<AliasMapping> getAliasMappings() {

        String identity = getDefaultMohUri();
        String contact = null;
        String mohSetting = getSipxFreeswitchService().getSettings().getSetting(
                SipxFreeswitchService.FREESWITCH_MOH_SOURCE).getValue();

        switch (SystemMohSetting.parseSetting(mohSetting)) {
        case FILES_SRC:
            contact = getLocalFilesMohUri();
            break;
        case SOUNDCARD_SRC:
            contact = getPortAudioMohUri();
            break;
        case LEGACY_PARK_MUSIC:
            break;
        default:
        }

        if (null != contact) {
            List<AliasMapping> aliasMappings = new ArrayList<AliasMapping>(1);
            aliasMappings.add(new AliasMapping(identity, contact));

            return aliasMappings;
        }

        return CollectionUtils.EMPTY_COLLECTION;
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

    public String getPersonalMohFilesUri(String userName) {
        return SipUri.format(m_personalMohFilesPrefix + userName, getSipxFreeswitchService().getDomainName(), false);
    }

    public String getPortAudioMohUri() {
        return SipUri.format(m_portAudioMohUser, getSipxFreeswitchService().getDomainName(), false);
    }

    public String getLocalFilesMohUri() {
        return SipUri.format(m_localFilesMohUser, getSipxFreeswitchService().getDomainName(), false);
    }

    public String getDefaultMohUri() {
        return SipUri.format(m_mohUser, getSipxFreeswitchService().getDomainName(), false);
    }

    @Required
    public void setAudioDirectory(String audioDirectory) {
        m_audioDirectory = audioDirectory;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setReplicationContext(SipxReplicationContext replicationContext) {
        m_replicationContext = replicationContext;
    }

    @Required
    public void setServiceConfigurator(ServiceConfigurator serviceConfigurator) {
        m_serviceConfigurator = serviceConfigurator;
    }

    @Required
    public void setMohUser(String mohUser) {
        m_mohUser = mohUser;
    }

    @Required
    public void setLocalFilesMohUser(String localFilesMohUser) {
        m_localFilesMohUser = localFilesMohUser;
    }

    @Required
    public void setPortAudioMohUser(String portAudioMohUser) {
        m_portAudioMohUser = portAudioMohUser;
    }

    @Required
    public void setPersonalMohFilesPrefix(String personalMohFilesPrefix) {
        m_personalMohFilesPrefix = personalMohFilesPrefix;
    }

    private SipxFreeswitchService getSipxFreeswitchService() {
        return (SipxFreeswitchService) m_sipxServiceManager.getServiceByBeanId(SipxFreeswitchService.BEAN_ID);
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
}
