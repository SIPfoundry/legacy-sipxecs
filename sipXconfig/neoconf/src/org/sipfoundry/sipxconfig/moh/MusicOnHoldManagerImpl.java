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
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
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
    public static final String LOCAL_FILES_SOURCE_SUFFIX = "l";
    public static final String PORT_AUDIO_SOURCE_SUFFIX = "p";
    public static final String USER_FILES_SOURCE_SUFFIX = "u";
    public static final String NONE_SUFFIX = "n";

    public static final Log LOG = LogFactory.getLog(MusicOnHoldManagerImpl.class);

    private static final String MOH = "moh";

    private SipxServiceManager m_sipxServiceManager;
    private String m_audioDirectory;
    private SipxReplicationContext m_replicationContext;
    private ServiceConfigurator m_serviceConfigurator;

    private String m_mohUser;

    /**
     * Music on hold implementation requires that ~~mh~u calls are forwarded to Media Server. We
     * are adding the rule here.
     */
    public List<DialingRule> getDialingRules() {
        DialingRule rule = new MohRule(getSipxFreeswitchAddressAndPort(), m_mohUser + USER_FILES_SOURCE_SUFFIX);
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
        String contact = null;
        String mohSetting = getSipxFreeswitchService().getSettingValue(SipxFreeswitchService.FREESWITCH_MOH_SOURCE);

        switch (SystemMohSetting.parseSetting(mohSetting)) {
        case SOUNDCARD_SRC:
            contact = getPortAudioMohUriMapping();
            break;
        case NONE:
            contact = getNoneMohUriMapping();
            break;
        case FILES_SRC:
        default:
            contact = getLocalFilesMohUriMapping();
            break;
        }

        List<AliasMapping> aliasMappings = new ArrayList<AliasMapping>(1);
        aliasMappings.add(new AliasMapping(getDefaultMohUri(), contact));

        aliasMappings.add(new AliasMapping(getLocalFilesMohUri(), getLocalFilesMohUriMapping()));
        aliasMappings.add(new AliasMapping(getPortAudioMohUri(), getPortAudioMohUriMapping()));
        aliasMappings.add(new AliasMapping(getNoneMohUri(), getNoneMohUriMapping()));

        return aliasMappings;
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
        return getMohUri(m_mohUser + USER_FILES_SOURCE_SUFFIX + userName);
    }

    public String getPortAudioMohUri() {
        return getMohUri(m_mohUser + PORT_AUDIO_SOURCE_SUFFIX);
    }

    public String getLocalFilesMohUri() {
        return getMohUri(m_mohUser + LOCAL_FILES_SOURCE_SUFFIX);
    }

    public String getNoneMohUri() {
        return getMohUri(m_mohUser + NONE_SUFFIX);
    }

    /**
     * The Moh URI used by the system
     */
    public String getDefaultMohUri() {
        return getMohUri(m_mohUser);
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

    private String getPortAudioMohUriMapping() {
        return getMohUriMapping(PORT_AUDIO_SOURCE_SUFFIX);
    }

    private String getLocalFilesMohUriMapping() {
        return getMohUriMapping(LOCAL_FILES_SOURCE_SUFFIX);
    }

    private String getNoneMohUriMapping() {
        return getMohUriMapping(NONE_SUFFIX);
    }

    /**
     * Build an alias which maps directly to the MOH server
     *
     * IVR@{FS}:{FSPort};action=moh; add "moh=l" for localstream files add "moh=p" for portaudio
     * (sound card) add "moh=u{username} for personal audio files add "moh=n" for disabled MOH
     *
     */
    private String getMohUriMapping(String mohParam) {
        Map<String, String> params = new LinkedHashMap<String, String>();
        params.put("action", MOH);
        if (mohParam != null) {
            params.put(MOH, mohParam);
        }
        return SipUri.format("IVR", getSipxFreeswitchAddressAndPort(), params);
    }

    private String getMohUri(String mohParam) {
        return SipUri.format(mohParam, getSipxFreeswitchService().getDomainName(), false);
    }

    private String getSipxFreeswitchAddressAndPort() {
        SipxFreeswitchService service = getSipxFreeswitchService();
        String host;
        if (service.getAddresses().size() > 1) {
            // HACK: this assumes that one of the freeswitch instances runs on a primary location
            // (but that neeeds to be true in order for MOH to work anyway)
            host = service.getLocationsManager().getPrimaryLocation().getAddress();
        } else {
            host = service.getAddress();
        }

        return host + ":" + service.getFreeswitchSipPort();
    }
}
