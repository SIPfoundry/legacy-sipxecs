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
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;

public class MusicOnHoldManagerImpl implements MusicOnHoldManager, DaoEventListener, BeanFactoryAware {
    public static final Log LOG = LogFactory.getLog(MusicOnHoldManagerImpl.class);
    private String m_audioDirectory;
    private String m_mohUser;
    private ListableBeanFactory m_beanFactory;

    /**
     * Music on hold implementation requires that ~~mh~u calls are forwarded to Media Server. We
     * are adding the rule here.
     */
    public List<DialingRule> getDialingRules() {
        MohAddressFactory factory = getAddressFactory();
        DialingRule rule = new MohRule(factory.getMediaAddress().toString(), factory.getFilesMohUser());
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
}
