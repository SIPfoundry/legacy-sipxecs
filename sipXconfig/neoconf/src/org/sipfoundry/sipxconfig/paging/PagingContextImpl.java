/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.paging;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Properties;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.PagingRule;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;
import org.sipfoundry.sipxconfig.setting.ConfigFileWriter;

public class PagingContextImpl extends SipxHibernateDaoSupport implements PagingContext {
    private static final String DESCRIPTION_KEY_FORMAT = "page.group.%d.description";
    private static final String BEEP_KEY_FORMAT = "page.group.%d.beep";
    private static final String USER_KEY_FORMAT = "page.group.%d.user";
    private static final String URLS_KEY_FORMAT = "page.group.%d.urls";

    private static final Log LOG = LogFactory.getLog(PagingContextImpl.class);

    private CoreContext m_coreContext;

    private String m_etcDirectory;

    private String m_audioDirectory;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setEtcDirectory(String etcDirectory) {
        m_etcDirectory = etcDirectory;
    }

    public String getAudioDirectory() {
        return m_audioDirectory;
    }

    public void setAudioDirectory(String directory) {
        m_audioDirectory = directory;
    }

    public String getPagingPrefix() {
        List pagingGroups = getHibernateTemplate().loadAll(PagingGroup.class);
        String prefix = "";
        if (pagingGroups.size() > 0) {
            PagingGroup pagingGroup = (PagingGroup) pagingGroups.get(0);
            prefix = pagingGroup.getPrefix();
        }
        return prefix;
    }

    public List<PagingGroup> getPagingGroups() {
        return getHibernateTemplate().loadAll(PagingGroup.class);
    }

    public PagingGroup getPagingGroupById(Integer pagingGroupId) {
        return (PagingGroup) getHibernateTemplate().load(PagingGroup.class, pagingGroupId);
    }

    public void savePagingPrefix(String prefix) {
        for (PagingGroup group : getPagingGroups()) {
            group.setPrefix(prefix);
            getHibernateTemplate().saveOrUpdate(group);
        }
    }

    public void savePagingGroup(PagingGroup group) {
        if (group.getPrefix() == null) {
            group.setPrefix(getPagingPrefix());
        }
        getHibernateTemplate().saveOrUpdate(group);
        // flush to take effect in order to generate the page group config
        getHibernateTemplate().flush();
        generatePageGroupConfig();
    }

    public void deletePagingGroupsById(Collection<Integer> groupsIds) {
        if (groupsIds.isEmpty()) {
            // no groups to delete => nothing to do
            return;
        }
        List<PagingGroup> groups = new ArrayList<PagingGroup>();
        for (Integer groupId : groupsIds) {
            groups.add(getPagingGroupById(groupId));
        }
        getHibernateTemplate().deleteAll(groups);
        // flush to take effect in order to generate the page group config
        getHibernateTemplate().flush();
        generatePageGroupConfig();
    }

    public void clear() {
        Collection c = getHibernateTemplate().loadAll(PagingGroup.class);
        getHibernateTemplate().deleteAll(c);
    }
    
    public List<DialingRule> getDialingRules() {
        String prefix = getPagingPrefix();
        List<DialingRule> pagingRules = new ArrayList<DialingRule>();
        if (!StringUtils.isEmpty(prefix)) {
            pagingRules.add(new PagingRule(prefix));
        }
        return pagingRules;
    }

    private synchronized void generatePageGroupConfig() {
        File configFile = new File(m_etcDirectory, "sipxpage.properties.in");
        try {
            ConfigFileWriter configWriter = new ConfigFileWriter(configFile);
            configWriter.reset();
            configWriter.store(getConfigProperties());
        } catch (IOException ex) {
            LOG.error("failed to write sipxpage.properties.in");
        }
    }

    private Properties getConfigProperties() {
        Properties configPageProperties = new Properties();
        String sipPort = "${PAGE_SERVER_SIP_PORT}";
        configPageProperties.setProperty("log.level", "NOTICE");
        configPageProperties.setProperty("log.file", "${PAGE_LOG_DIR}/sipxpage.log");
        configPageProperties.setProperty("sip.address", "${PAGE_SERVER_ADDR}");
        configPageProperties.setProperty("sip.udpPort", sipPort);
        configPageProperties.setProperty("sip.tcpPort", sipPort);
        configPageProperties.setProperty("sip.tlsPort", "${PAGE_SERVER_SIP_SECURE_PORT}");
        configPageProperties.setProperty("rtp.port", "8500");
        int pagingGroupIndex = 1;
        for (PagingGroup group : getPagingGroups()) {
            if (group.isEnabled()) {
                configPageProperties.setProperty(String.format(DESCRIPTION_KEY_FORMAT,
                        pagingGroupIndex), group.formatDescription());
                configPageProperties.setProperty(
                        String.format(BEEP_KEY_FORMAT, pagingGroupIndex), group
                                .formatBeep(m_audioDirectory));
                configPageProperties.setProperty(
                        String.format(USER_KEY_FORMAT, pagingGroupIndex), group
                                .formatPageGroupNumber());
                configPageProperties.setProperty(
                        String.format(URLS_KEY_FORMAT, pagingGroupIndex), group
                                .formatUrls(m_coreContext.getDomainName()));
                pagingGroupIndex++;
            }
        }
        return configPageProperties;
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            List<PagingGroup> groups = getPagingGroups();
            for (PagingGroup group : groups) {
                Set<User> users = group.getUsers();
                if (users.remove(user)) {
                    getHibernateTemplate().saveOrUpdate(group);
                }
            }
            generatePageGroupConfig();
        }
    }
}
