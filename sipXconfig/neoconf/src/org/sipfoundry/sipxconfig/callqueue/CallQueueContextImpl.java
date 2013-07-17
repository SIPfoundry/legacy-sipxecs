/*
 * Copyright (C) 2013 SibTelCom, JSC., certain elements licensed under a Contributor Agreement.
 * Author: Konstantin S. Vishnivetsky
 * E-mail: info@siplabs.ru
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.callqueue;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.hibernate.Query;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.feature.Bundle;
import org.sipfoundry.sipxconfig.feature.FeatureChangeRequest;
import org.sipfoundry.sipxconfig.feature.FeatureChangeValidator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureProvider;
import org.sipfoundry.sipxconfig.feature.GlobalFeature;
import org.sipfoundry.sipxconfig.feature.LocationFeature;

import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.SameExtensionException;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.common.ExtensionInUseException;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.imdb.ReplicationManager;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;

import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;

import org.sipfoundry.sipxconfig.freeswitch.FreeswitchCondition;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;

public class CallQueueContextImpl extends SipxHibernateDaoSupport implements CallQueueContext, BeanFactoryAware,
        FeatureProvider {

    private static final String QUERY_CALL_QUEUE_IDS = "callQueueIds";
    private static final String QUERY_CALL_QUEUE_AGENT_IDS = "callQueueAgentIds";
    private static final String QUERY_CALL_QUEUE_EXTENSIONS_WITH_NAMES = "callQueueExtensionWithName";
    private static final String QUERY_PARAM_VALUE = "value";
    private static final String QUERY_PARAM_AGENT_ID = "callqueueagentid";
    private static final String QUERY_PARAM_QUEUE_ID = "callqueueid";
    private static final String COPY_OF = "Copy of";
    private static final String COPIED = "(Copied)";

    private static final String ALIAS = "alias";
    private static final String EXTENSION = "extension";
    private static final String DID = "did";
    private static final String QUEUE_NAME = CALL_QUEUE;

    private BeanFactory m_beanFactory;
    private AliasManager m_aliasManager;
    private FeatureManager m_featureManager;
    private ReplicationManager m_replicationManager;
    private BeanWithSettingsDao<CallQueueSettings> m_settingsDao;

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    /* Bean properties */
    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    public void setReplicationManager(ReplicationManager replicationManager) {
        m_replicationManager = replicationManager;
    }

    /* Settings API */

    public void setSettingsDao(BeanWithSettingsDao<CallQueueSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    public CallQueueSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    public void saveSettings(CallQueueSettings settings) {
        m_settingsDao.upsert(settings);
        refreshCallQueueCommands(settings);
    }

    @Override
    public List<Replicable> getReplicables() {
        List<Replicable> replicables = new ArrayList<Replicable>();
        replicables.addAll(getCallQueues());
        replicables.addAll(getCallQueueAgents());
        replicables.addAll(getCallQueueCommands());
        return replicables;
    }

    /* Alias support */
    @Override
    public Collection<BeanId> getBeanIdsOfObjectsWithAlias(String alias) {
        Collection<BeanId> bids = new ArrayList<BeanId>();

        List<CallQueue> lines = getHibernateTemplate().loadAll(CallQueue.class);
        for (CallQueue line : lines) {
            if (line.getExtension() != null && (line.getExtension().equals(alias) || line.getName().equals(alias))
                    || (line.getAlias() != null && line.getAlias().equals(alias))
                    || (line.getDid() != null && line.getDid().equals(alias))) {
                bids.add(new BeanId(line.getId(), CallQueue.class));
            }
        }
        // Add all beans, having alias(es)
        return bids;
    }

    @Override
    public boolean isAliasInUse(String alias) {
        List<CallQueueExtension> extensions = getFreeswitchExtensions();
        for (CallQueueExtension extension : extensions) {
            if (extension.getExtension() != null
                    && (extension.getExtension().equals(alias) || extension.getName().equals(alias))) {
                return true;
            }
            if (extension.getAlias() != null && extension.getAlias().equals(alias)) {
                return true;
            }
            if (extension.getDid() != null && extension.getDid().equals(alias)) {
                return true;
            }
        }
        return false;
    }

    /* FreeSwitchExtensionProvider */
    @Override
    public boolean isEnabled() {
        return m_featureManager.isFeatureEnabled(FEATURE);
    }

    public void deleteExtension(CallQueueExtension ext) {
        getHibernateTemplate().delete(ext);
        getHibernateTemplate().flush();
    }

    public void saveExtension(CallQueueExtension extension) {
        if (extension.getName() == null) {
            throw new UserException("&null.name");
        }
        if (extension.getExtension() == null) {
            throw new UserException("&null.extension");
        }
        String capturedExt = extension.getCapturedExtension();

        if (!m_aliasManager.canObjectUseAlias(extension, extension.getName())) {
            throw new NameInUseException(QUEUE_NAME, extension.getName());
        } else if (!m_aliasManager.canObjectUseAlias(extension, capturedExt)) {
            throw new ExtensionInUseException(QUEUE_NAME, capturedExt);
        } else if (extension.getAlias() != null
                && !m_aliasManager.canObjectUseAlias(extension, extension.getAlias())) {
            throw new ExtensionInUseException(QUEUE_NAME, extension.getAlias());
        } else if (extension.getAlias() != null && extension.getAlias().equals(extension.getExtension())) {
            throw new SameExtensionException(ALIAS, EXTENSION);
        } else if (extension.getDid() != null && !m_aliasManager.canObjectUseAlias(extension, extension.getDid())) {
            throw new ExtensionInUseException(QUEUE_NAME, extension.getDid());
        } else if (extension.getDid() != null && extension.getDid().equals(extension.getExtension())) {
            throw new SameExtensionException(DID, EXTENSION);
        } else if (extension.getDid() != null && extension.getAlias() != null
                && extension.getDid().equals(extension.getAlias())) {
            throw new SameExtensionException(ALIAS, DID);
        }
        removeNullActions(extension);
        if (extension.isNew()) {
            getHibernateTemplate().saveOrUpdate(extension);
        } else {
            getHibernateTemplate().merge(extension);
        }
    }

    @Override
    public CallQueueExtension getExtensionById(Integer extensionId) {
        return getHibernateTemplate().load(CallQueueExtension.class, extensionId);
    }

    @Override
    public CallQueueExtension getExtensionByName(String extensionName) {
        List<CallQueue> extensions = getHibernateTemplate().findByNamedQueryAndNamedParam(
                QUERY_CALL_QUEUE_EXTENSIONS_WITH_NAMES, QUERY_PARAM_VALUE, extensionName);
        return DataAccessUtils.singleResult(extensions);
    }

    @Override
    public List<CallQueueExtension> getFreeswitchExtensions() {
        return getHibernateTemplate().loadAll(CallQueueExtension.class);
    }

    private void removeNullActions(CallQueueExtension extension) { // Should not be Tested
        if (extension.getConditions() == null) {
            return;
        }
        for (FreeswitchCondition condition : extension.getConditions()) {
            for (FreeswitchAction action : condition.getActions()) {
                if (action != null && action.getApplication() == null) {
                    condition.removeAction(action);
                }
            }
        }
    }

    /* CallQueue API */

    public CallQueue newCallQueue() { // Tested
        CallQueue callqueue = (CallQueue) m_beanFactory.getBean(CallQueue.class);
        return callqueue;
    }

    public void storeCallQueue(CallQueue callQueue) { // Tested
        getDaoEventPublisher().publishSave(callQueue);
        saveExtension(callQueue);
    }

    public CallQueue loadCallQueue(Integer id) { // Tested
        return (CallQueue) getHibernateTemplate().load(CallQueue.class, id);
    }

    public void duplicateCallQueues(Collection<Integer> ids) { // Tested
        for (Integer id : ids) {
            CallQueue srcCallQueue = (CallQueue) getHibernateTemplate().load(CallQueue.class, id);
            CallQueue newCallQueue = newCallQueue();
            // TODO: localize strings
            newCallQueue.setName(COPY_OF + srcCallQueue.getName());
            if (null != srcCallQueue.getDescription()) {
                newCallQueue.setDescription(srcCallQueue.getDescription() + COPIED);
            }
            srcCallQueue.copySettingsTo(newCallQueue);
            getDaoEventPublisher().publishSave(newCallQueue);
            getHibernateTemplate().saveOrUpdate(newCallQueue);
        }
    }

    public void removeCallQueues(Collection<Integer> ids) { // Tested
        if (ids.isEmpty()) {
            return;
        }
        removeAll(CallQueue.class, ids);
    }

    public Collection<CallQueue> getCallQueues() { // Test
        return getHibernateTemplate().loadAll(CallQueue.class);
    }

    /* CallQueueCommand API */

    private void createCallQeuueCommand(String command, String status) { // Should not be Tested
        CallQueueCommand callQueueCommand = newCallQueueCommand();
        callQueueCommand.setName(command);
        callQueueCommand.setAlias(getSettings().getSettingValue("call-queue-agent-code/" + command));
        callQueueCommand.setExtension(command, status);
        storeCallQueueCommand(callQueueCommand);
    }

    private void refreshCallQueueCommands(CallQueueSettings settings) { // Should not be Tested
        Collection<CallQueueCommand> callqueuecommands = getCallQueueCommands();
        for (CallQueueCommand callQueueCommand : callqueuecommands) {
            callQueueCommand.setAlias(settings.getSettingValue(String.format("call-queue-agent-code/%s",
                    callQueueCommand.getName())));
            storeCallQueueCommand(callQueueCommand);
        }
    }

    private CallQueueCommand newCallQueueCommand() { // Should not be Tested
        CallQueueCommand callqueuecommand = (CallQueueCommand) m_beanFactory.getBean(CallQueueCommand.class);
        return callqueuecommand;
    }

    private void storeCallQueueCommand(CallQueueCommand callQueueCommand) { // Should not be
                                                                            // Tested
        getDaoEventPublisher().publishSave(callQueueCommand);
        saveExtension(callQueueCommand);
    }

    private CallQueueCommand loadCallQueueCommand(Integer id) { // Should not be Tested
        return (CallQueueCommand) getHibernateTemplate().load(CallQueueCommand.class, id);
    }

    private Collection<CallQueueCommand> getCallQueueCommands() { // Should not be Tested
        return getHibernateTemplate().loadAll(CallQueueCommand.class);
    }

    /* CallQueueAgent API */

    public CallQueueAgent newCallQueueAgent() { // Tested
        CallQueueAgent callqueueagent = (CallQueueAgent) m_beanFactory.getBean(CallQueueAgent.class);
        return callqueueagent;
    }

    public void storeCallQueueAgent(CallQueueAgent callQueueAgent) { // Tested
        // Check for duplicate names and extensions before saving the call queue
        String name = callQueueAgent.getName();
        final String callQueueAgentTypeName = "&label.callQueueAgent";
        if (!m_aliasManager.canObjectUseAlias(callQueueAgent, name)) {
            throw new NameInUseException(callQueueAgentTypeName, name);
        }

        getDaoEventPublisher().publishSave(callQueueAgent);
        getHibernateTemplate().saveOrUpdate(callQueueAgent);
    }

    public CallQueueAgent loadCallQueueAgent(Integer id) { // Tested
        return (CallQueueAgent) getHibernateTemplate().load(CallQueueAgent.class, id);
    }

    public void duplicateCallQueueAgents(Collection<Integer> ids) { // Tested
        for (Integer id : ids) {
            CallQueueAgent srcCallQueueAgent = (CallQueueAgent) getHibernateTemplate()
                    .load(CallQueueAgent.class, id);
            CallQueueAgent newCallQueueAgent = newCallQueueAgent();
            // TODO: localize strings
            newCallQueueAgent.setName(COPY_OF + srcCallQueueAgent.getName());
            if (null != srcCallQueueAgent.getDescription()) {
                newCallQueueAgent.setDescription(srcCallQueueAgent.getDescription() + COPIED);
            }
            srcCallQueueAgent.copySettingsTo(newCallQueueAgent);
            getDaoEventPublisher().publishSave(newCallQueueAgent);
            getHibernateTemplate().saveOrUpdate(newCallQueueAgent);
        }
    }

    public void removeCallQueueAgents(Collection<Integer> ids) { // Tested
        if (ids.isEmpty()) {
            return;
        }
        removeAll(CallQueueAgent.class, ids);
    }

    public Collection<CallQueueAgent> getCallQueueAgents() { // Tested
        return getHibernateTemplate().loadAll(CallQueueAgent.class);
    }

    @Override
    public List<CallQueue> getAvaiableQueuesForAgent(Integer callqueueagentid) { // Tested
        if (null == callqueueagentid) {
            return Collections.EMPTY_LIST;
        }
        Query query = getSession()
                .createSQLQuery(
                        "(SELECT q.* FROM freeswitch_extension q WHERE freeswitch_ext_type = 'q')" + " EXCEPT"
                                + " (SELECT DISTINCT q.* FROM freeswitch_extension q"
                                + " LEFT JOIN call_queue_tier t USING(freeswitch_ext_id)"
                                + " WHERE t.call_queue_agent_id = :" + QUERY_PARAM_AGENT_ID + ")")
                .addEntity(CallQueue.class).setParameter(QUERY_PARAM_AGENT_ID, callqueueagentid.intValue());
        List<CallQueue> result = query.list();
        return result;
    }

    @Override
    public List<Integer> getCallQueueAgentsForQueue(Integer callqueueid) { // Tested
        if (null == callqueueid) {
            return Collections.EMPTY_LIST;
        }
        Query query = getSession().createSQLQuery(
                "SELECT DISTINCT t.call_queue_agent_id FROM call_queue_tier t WHERE t.freeswitch_ext_id = :"
                        + QUERY_PARAM_QUEUE_ID).setParameter(QUERY_PARAM_QUEUE_ID, callqueueid.intValue());
        List<Integer> result = query.list();
        return result;
    }

    @Override
    public void featureChangePrecommit(FeatureManager manager, FeatureChangeValidator validator) {
        validator.requiredOnSameHost(FEATURE, FreeswitchFeature.FEATURE);
        validator.singleLocationOnly(FEATURE);
        Collection<CallQueueCommand> callqueuecommands = getCallQueueCommands();
        // TODO: make it right...
        if (callqueuecommands.size() == 0) {
            createCallQeuueCommand("agent-login", "Available");
            createCallQeuueCommand("agent-on-break", "On Break");
            createCallQeuueCommand("agent-logout", "Logged Out");
        }
    }

    @Override
    public void featureChangePostcommit(FeatureManager manager, FeatureChangeRequest request) {
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

}
