/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.alias.AliasManager;
import org.sipfoundry.sipxconfig.common.BeanId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.DidInUseException;
import org.sipfoundry.sipxconfig.common.ExtensionInUseException;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class AcdContextImpl extends SipxHibernateDaoSupport implements AcdContext, BeanFactoryAware,
        DaoEventListener, ProcessProvider {
    public static final Log LOG = LogFactory.getLog(AcdContextImpl.class);
    private static final String NAME_PROPERTY = "name";
    private static final String SERVER_PARAM = "acdServer";
    private static final String USER_PARAM = "user";
    private static final String SQL = "alter table acd_server drop column host";
    private static final String ACD_LINE_IDS_WITH_ALIAS = "acdLineIdsWithAlias";
    private static final String VALUE = "value";
    private static final String LINE = "&label.line";
    private static final String AGENT_FOR_USER_AND_SERVER_QUERY = "agentForUserAndServer";

    private AliasManager m_aliasManager;
    private BeanFactory m_beanFactory;
    private LocationsManager m_locationsManager;
    private CoreContext m_coreContext;
    private FeatureManager m_featureManager;
    private boolean m_enabled;

    private AcdServer getAcdServer(Integer id) {
        return (AcdServer) getHibernateTemplate().load(AcdServer.class, id);
    }

    @Override
    public List<AcdServer> getServers() {
        return getHibernateTemplate().loadAll(AcdServer.class);
    }

    @Override
    public List<AcdLine> getLines() {
        return getHibernateTemplate().loadAll(AcdLine.class);
    }

    @Override
    public boolean isAcdServerIdValid(int acdServerId) {
        List<AcdServer> acdServers = getServers();
        for (AcdServer acdServer : acdServers) {
            if (acdServer.getId() == acdServerId) {
                return true;
            }
        }
        return false;
    }

    @Override
    public List getUsersWithAgents() {
        return getHibernateTemplate().findByNamedQuery("usersWithAgents");
    }

    @Override
    public List getUsersWithAgentsForLocation(Location location) {
        AcdServer acdServer = getAcdServerForLocationId(location.getId());
        return getHibernateTemplate().findByNamedQueryAndNamedParam("usersWithAgentsForServer", SERVER_PARAM,
                acdServer);
    }

    @Override
    public AcdServer loadServer(Serializable id) {
        return (AcdServer) getHibernateTemplate().load(AcdServer.class, id);
    }

    @Override
    public void saveComponent(AcdComponent acdComponent) {
        if (acdComponent instanceof AcdLine) {
            AcdLine line = (AcdLine) acdComponent;
            String name = line.getName();
            String extension = line.getExtension();
            String did = line.getDid();
            DaoUtils.checkDuplicates(getHibernateTemplate(), AcdLine.class, line, NAME_PROPERTY,
                    new NameInUseException(LINE, line.getName()));
            DaoUtils.checkDuplicates(getHibernateTemplate(), AcdLine.class, line, "extension",
                    new ExtensionInUseException(LINE, line.getExtension()));

            if (!m_aliasManager.canObjectUseAlias(line, name)) {
                throw new NameInUseException(LINE, name);
            }
            if (!m_aliasManager.canObjectUseAlias(line, extension)) {
                throw new ExtensionInUseException(LINE, extension);
            }
            if (!m_aliasManager.canObjectUseAlias(line, did)) {
                throw new ExtensionInUseException(LINE, did);
            }
            if (StringUtils.isNotBlank(did) && did.equals(extension)) {
                throw new DidInUseException(LINE, did);
            }
        }
        if (acdComponent instanceof AcdQueue) {
            AcdQueue queue = (AcdQueue) acdComponent;

            DaoUtils.checkDuplicates(getHibernateTemplate(), AcdQueue.class, queue, NAME_PROPERTY,
                    new NameInUseException("queue", queue.getName()));
        }
        if (!acdComponent.isNew()) {
            getHibernateTemplate().merge(acdComponent);
        } else {
            getHibernateTemplate().save(acdComponent);
        }
        getDaoEventPublisher().publishSave(acdComponent);
    }

    @Override
    public AcdLine loadLine(Serializable id) {
        return (AcdLine) getHibernateTemplate().load(AcdLine.class, id);
    }

    @Override
    public AcdQueue loadQueue(Serializable id) {
        return (AcdQueue) getHibernateTemplate().load(AcdQueue.class, id);
    }

    @Override
    public AcdAgent loadAgent(Serializable id) {
        return (AcdAgent) getHibernateTemplate().load(AcdAgent.class, id);
    }

    @Override
    public AcdAudio newAudio() {
        return (AcdAudio) m_beanFactory.getBean(AcdAudio.BEAN_NAME, AcdAudio.class);
    }

    @Override
    public AcdLine newLine() {
        return (AcdLine) m_beanFactory.getBean(AcdLine.BEAN_NAME, AcdLine.class);
    }

    @Override
    public AcdServer newServer() {
        return (AcdServer) m_beanFactory.getBean(AcdServer.BEAN_NAME, AcdServer.class);
    }

    @Override
    public void removeServers(Collection<AcdServer> servers) {
        HibernateTemplate hibernate = getHibernateTemplate();
        hibernate.deleteAll(servers);
    }

    @Override
    public void removeServersByIds(Collection serversIds) {
        HibernateTemplate hibernate = getHibernateTemplate();
        Collection servers = new ArrayList();
        for (Iterator i = serversIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            AcdServer server = loadServer(id);
            servers.add(server);
        }
        getDaoEventPublisher().publishDelete(servers);
        hibernate.deleteAll(servers);
    }

    @Override
    public void removeLines(Collection linesIds) {
        HibernateTemplate hibernate = getHibernateTemplate();
        Collection servers = new HashSet();
        for (Iterator i = linesIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            AcdLine line = (AcdLine) hibernate.load(AcdLine.class, id);
            AcdServer acdServer = line.getAcdServer();
            line.associateQueue(null);
            acdServer.removeLine(line);
            getDaoEventPublisher().publishDelete(line);
            servers.add(acdServer);
        }
        getDaoEventPublisher().publishSave(servers);
        hibernate.saveOrUpdateAll(servers);
    }

    @Override
    public AcdQueue newQueue() {
        return (AcdQueue) m_beanFactory.getBean(AcdQueue.BEAN_NAME, AcdQueue.class);
    }

    @Override
    public void removeQueues(Collection queuesIds) {
        HibernateTemplate hibernate = getHibernateTemplate();
        for (Iterator i = queuesIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            AcdQueue queue = (AcdQueue) hibernate.load(AcdQueue.class, id);
            AcdServer acdServer = queue.getAcdServer();
            acdServer.removeQueue(queue);
            getDaoEventPublisher().publishDelete(queue);
            queue.cleanLines();
            queue.cleanAgents();
            cleanReferencesToOverflowQueue(queue);
        }
        removeOverflowSettings(queuesIds, AcdQueue.QUEUE_TYPE);
        // we need to save server, agents and queues - the easiest thing to do is to flush
        hibernate.flush();
    }

    private void cleanReferencesToOverflowQueue(AcdQueue overflowQueue) {
        List queues = getHibernateTemplate().findByNamedQueryAndNamedParam("queuesForOverflowQueue",
                "overflowQueue", overflowQueue);
        for (Iterator i = queues.iterator(); i.hasNext();) {
            AcdQueue queue = (AcdQueue) i.next();
            queue.setOverflowQueue(null);
        }
    }

    private AcdAgent newAgent(AcdServer server, User user) {
        String[] params = {
            USER_PARAM, SERVER_PARAM
        };
        Object[] values = {
            user, server
        };
        List agents = getHibernateTemplate().findByNamedQueryAndNamedParam(AGENT_FOR_USER_AND_SERVER_QUERY, params,
                values);
        if (!agents.isEmpty()) {
            return (AcdAgent) agents.get(0);
        }
        AcdAgent agent = (AcdAgent) m_beanFactory.getBean(AcdAgent.BEAN_NAME, AcdAgent.class);
        agent.setUser(user);
        // FIXME: agents need to be saved before they are added to server's collections but one
        // cannot save the agent without setting its server field
        server.insertAgent(agent);
        return agent;
    }

    @Override
    public boolean isUserAnAgentOnThisServer(AcdServer server, User user) {
        String[] params = {
            USER_PARAM, SERVER_PARAM
        };
        Object[] values = {
            user, server
        };
        List agents = getHibernateTemplate().findByNamedQueryAndNamedParam(AGENT_FOR_USER_AND_SERVER_QUERY, params,
                values);
        return !agents.isEmpty();
    }

    @Override
    public void removeAgents(Serializable acdQueueId, Collection agentsIds) {
        AcdQueue queue = loadQueue(acdQueueId);
        AcdServer server = queue.getAcdServer();
        HibernateTemplate hibernate = getHibernateTemplate();

        for (Iterator i = agentsIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            AcdAgent agent = loadAgent(id);
            queue.removeAgent(agent);
            if (agent.getQueues().isEmpty()) {
                server.removeAgent(agent);
            }
        }
        getDaoEventPublisher().publishSave(server);
        hibernate.save(server);
        getDaoEventPublisher().publishSave(queue);
        hibernate.save(queue);
    }

    @Override
    public void addUsersToQueue(Serializable queueId, Collection usersIds) {
        AcdQueue queue = loadQueue(queueId);
        CoreContext coreContext = queue.getCoreContext();
        AcdServer server = queue.getAcdServer();

        for (Iterator i = usersIds.iterator(); i.hasNext();) {
            Integer userId = (Integer) i.next();
            User user = coreContext.loadUser(userId);
            AcdAgent agent = newAgent(server, user);
            getHibernateTemplate().save(agent);
            queue.insertAgent(agent);
        }
        getHibernateTemplate().save(server);
        getHibernateTemplate().save(queue);
    }

    public void associate(Serializable lineId, Serializable queueId) {
        AcdLine line = loadLine(lineId);
        AcdQueue newQueue = null;
        if (queueId != null) {
            newQueue = loadQueue(queueId);
        }
        AcdQueue oldQueue = line.associateQueue(newQueue);
        getHibernateTemplate().update(line);
        // TODO: maybe we need to fix cascading in hibernate
        // cascade all does save new queue but not the old one
        if (oldQueue != null) {
            getHibernateTemplate().update(oldQueue);
        }
    }

    public Collection<AliasMapping> getAliasMappings() {
        if (!m_featureManager.isFeatureEnabled(Acd.FEATURE)) {
            return null;
        }
        HibernateTemplate hibernate = getHibernateTemplate();
        List<AcdLine> acdLines = hibernate.loadAll(AcdLine.class);
        Collection<AliasMapping> aliases = new ArrayList<AliasMapping>();
        for (AcdLine acdLine : acdLines) {
            aliases.addAll(acdLine.getAliasMappings(m_coreContext.getDomainName()));
        }

        return aliases;
    }

    @Override
    public void moveAgentsInQueue(Serializable queueId, Collection agentsIds, int step) {
        AcdQueue queue = loadQueue(queueId);
        queue.moveAgents(agentsIds, step);
        saveComponent(queue);
    }

    @Override
    public void moveQueuesInAgent(Serializable agnetId, Collection queueIds, int step) {
        AcdAgent agent = loadAgent(agnetId);
        agent.moveQueues(queueIds, step);
        saveComponent(agent);
    }

    @Override
    public void clear() {
        // only need to delete servers and agents - lines and queues are handled by cascades
        List components = getHibernateTemplate().loadAll(AcdServer.class);
        components.addAll(getHibernateTemplate().loadAll(AcdAgent.class));
        getHibernateTemplate().deleteAll(components);
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    private void onUserDelete(User user) {
        // FIXME: associate user with the session...
        getHibernateTemplate().update(user);
        List agents = getHibernateTemplate().findByNamedQueryAndNamedParam("agentForUser", USER_PARAM, user);
        Set servers = new HashSet();
        for (Iterator i = agents.iterator(); i.hasNext();) {
            AcdAgent agent = (AcdAgent) i.next();
            agent.removeFromAllQueues();
            AcdServer server = agent.getAcdServer();
            server.removeAgent(agent);
            servers.add(server);
            // HACK: cascade should work here (server delete should delete orphan agents...
            getHibernateTemplate().delete(agent);
        }
        getHibernateTemplate().saveOrUpdateAll(servers);
    }

    @Override
    public String getAudioServerUrl() {
        String audioServerAddr = m_locationsManager.getPrimaryLocation().getAddress();
        return "http://" + audioServerAddr + "/phone/acd/audio";
    }

    @Override
    public void migrateOverflowQueues() {
        List queues = getHibernateTemplate().loadAll(AcdQueue.class);
        Map name2queue = new HashMap(queues.size());
        List queuesToBeFixed = new ArrayList(queues.size());

        for (Iterator i = queues.iterator(); i.hasNext();) {
            AcdQueue queue = (AcdQueue) i.next();
            name2queue.put(queue.getName(), queue);
            String overflowQueueName = queue.getSettingValue(AcdQueue.OVERFLOW_QUEUE);
            if (StringUtils.isNotBlank(overflowQueueName)) {
                queuesToBeFixed.add(queue);
            }
        }
        for (Iterator i = queuesToBeFixed.iterator(); i.hasNext();) {
            AcdQueue queue = (AcdQueue) i.next();
            Setting overflowQueueUriSetting = queue.getSettings().getSetting(AcdQueue.OVERFLOW_QUEUE);
            String overflowQueueUri = overflowQueueUriSetting.getValue();
            String name = SipUri.extractUser(overflowQueueUri);
            AcdQueue overflowQueue = (AcdQueue) name2queue.get(name);
            if (overflowQueue != null) {
                queue.setOverflowQueue(overflowQueue);
                queue.getValueStorage().revertSettingToDefault(overflowQueueUriSetting);
            }
        }
        getHibernateTemplate().saveOrUpdateAll(queuesToBeFixed);
    }

    public void migrateLineExtensions() {
        List lines = getHibernateTemplate().loadAll(AcdLine.class);
        for (Iterator i = lines.iterator(); i.hasNext();) {
            AcdLine line = (AcdLine) i.next();
            Setting extensionSetting = line.getSettings().getSetting(AcdLine.EXTENSION);
            String extension = extensionSetting.getValue();
            if (StringUtils.isNotEmpty(extension)) {
                line.setExtension(extension);
                line.getValueStorage().revertSettingToDefault(extensionSetting);
            }
        }
        getHibernateTemplate().saveOrUpdateAll(lines);
    }

    @Override
    public Collection<AcdQueue> getQueuesForUsers(AcdServer server, Collection<User> agents) {
        if (agents.isEmpty()) {
            return Collections.EMPTY_LIST;
        }

        Collection agentIds = DataCollectionUtil.extractPrimaryKeys(agents);
        String[] params = new String[] {
            "userIds", "serverId"
        };
        Object[] values = new Object[] {
            agentIds, server.getId()
        };
        List queues = getHibernateTemplate().findByNamedQueryAndNamedParam("queuesForUsers", params, values);
        return queues;
    }

    @Override
    public void removeOverflowSettings(Collection overflowIds, String overflowType) {
        List<ValueStorage> l = getHibernateTemplate().findByNamedQuery("valueStorage");
        for (ValueStorage storage : l) {
            Map settings = storage.getDatabaseValues();

            String type = settings == null ? null : (String) settings.get(AcdQueue.OVERFLOW_TYPE);
            if (type != null && type.equals(overflowType)) {
                String queueId = settings.get(AcdQueue.OVERFLOW_TYPEVALUE).toString();
                for (Object id : overflowIds) {
                    if (id.toString().equals(queueId)) {
                        settings.remove(AcdQueue.OVERFLOW_TYPE);
                        settings.remove(AcdQueue.OVERFLOW_TYPEVALUE);
                    }
                }
            }
            getHibernateTemplate().saveOrUpdate(storage);
        }
    }

    @Override
    public void addNewServer(Location location) {
        // HACK: this probably should be only called from event notification
        AcdServer acdServer = getAcdServerForLocationId(location.getId());
        if (acdServer == null) {
            acdServer = newServer();
            acdServer.setLocation(location);
            saveComponent(acdServer);
        }
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Override
    public AcdServer getAcdServerForLocationId(Integer locationId) {
        HibernateTemplate hibernate = getHibernateTemplate();
        List<AcdServer> servers = hibernate.findByNamedQueryAndNamedParam("acdServerForLocationId", "locationId",
                locationId);

        return (AcdServer) DataAccessUtils.singleResult(servers);
    }

    @Override
    public boolean isAliasInUse(String alias) {
        if (!m_featureManager.isFeatureEnabled(Acd.FEATURE)) {
            return false;
        }
        for (AliasMapping aliasMapping : getAliasMappings()) {
            if (aliasMapping.getIdentity().equals(alias)) {
                return true;
            }
        }
        return false;
    }

    @Override
    public Collection getBeanIdsOfObjectsWithAlias(String alias) {
        Collection<Integer> ids = getHibernateTemplate().findByNamedQueryAndNamedParam(ACD_LINE_IDS_WITH_ALIAS,
                VALUE, alias);
        Collection<BeanId> bids = BeanId.createBeanIdCollection(ids, AcdLine.class);

        return bids;
    }

    @Required
    public void setAliasManager(AliasManager aliasManager) {
        m_aliasManager = aliasManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Override
    public void onDelete(Object entity) {
        if (entity instanceof User) {
            onUserDelete((User) entity);
        } else if (entity instanceof Location) {
            onLocationDelete((Location) entity);
        }
    }

    @Override
    public void onSave(Object entity) {
    }

    private void onLocationDelete(Location location) {
        AcdServer server = getAcdServerForLocationId(location.getId());
        if (server != null) {
            getHibernateTemplate().delete(server);
        }
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(Acd.FEATURE, location);
        return (enabled ? Collections.singleton(ProcessDefinition.sipx("sipxacd")) : null);
    }

    @Override
    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }
}
