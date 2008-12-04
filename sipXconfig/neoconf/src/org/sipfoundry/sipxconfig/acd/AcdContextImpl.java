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
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
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
import org.hibernate.classic.Session;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.service.LocationSpecificService;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class AcdContextImpl extends SipxHibernateDaoSupport implements AcdContext, BeanFactoryAware,
        DaoEventListener {
    public static final Log LOG = LogFactory.getLog(AcdContextImpl.class);

    private static final String NAME_PROPERTY = "name";

    private static final String SERVER_PARAM = "acdServer";

    private static final String USER_PARAM = "user";

    private static final String SQL = "alter table acd_server drop column host";

    private BeanFactory m_beanFactory;

    private String m_audioServerUrl;

    private boolean m_enabled = true;

    private LocationsManager m_locationsManager;

    private SipxServiceManager m_sipxServiceManager;

    private class NameInUseException extends UserException {
        private static final String ERROR = "The name \"{1}\" is already in use. "
                + "Please choose another name for this {0}.";

        public NameInUseException(String objectType, String name) {
            super(ERROR, objectType, name);
        }
    }

    private class ExtensionInUseException extends UserException {
        private static final String ERROR = "The extension \"{0}\" is already in use. "
                + "Please choose another extension for this line.";

        public ExtensionInUseException(String name) {
            super(ERROR, name);
        }
    }

    private AcdServer getAcdServer(Integer id) {
        return (AcdServer) getHibernateTemplate().load(AcdServer.class, id);
    }

    public List getServers() {
        return getHibernateTemplate().loadAll(AcdServer.class);
    }

    public List getUsersWithAgents() {
        return getHibernateTemplate().findByNamedQuery("usersWithAgents");
    }

    public AcdServer loadServer(Serializable id) {
        return (AcdServer) getHibernateTemplate().load(AcdServer.class, id);
    }

    public void store(AcdComponent acdComponent) {
        if (acdComponent instanceof AcdLine) {
            AcdLine line = (AcdLine) acdComponent;
            DaoUtils.checkDuplicates(getHibernateTemplate(), AcdLine.class, line, NAME_PROPERTY,
                    new NameInUseException("line", line.getName()));
            DaoUtils.checkDuplicates(getHibernateTemplate(), AcdLine.class, line, "extension",
                    new ExtensionInUseException(line.getExtension()));
        }
        if (acdComponent instanceof AcdQueue) {
            AcdQueue queue = (AcdQueue) acdComponent;

            DaoUtils.checkDuplicates(getHibernateTemplate(), AcdQueue.class, queue, NAME_PROPERTY,
                    new NameInUseException("queue", queue.getName()));
        }

        getHibernateTemplate().saveOrUpdate(acdComponent);
        getHibernateTemplate().flush();
    }

    public AcdLine loadLine(Serializable id) {
        return (AcdLine) getHibernateTemplate().load(AcdLine.class, id);
    }

    public AcdQueue loadQueue(Serializable id) {
        return (AcdQueue) getHibernateTemplate().load(AcdQueue.class, id);
    }

    public AcdAgent loadAgent(Serializable id) {
        return (AcdAgent) getHibernateTemplate().load(AcdAgent.class, id);
    }

    public AcdAudio newAudio() {
        return (AcdAudio) m_beanFactory.getBean(AcdAudio.BEAN_NAME, AcdAudio.class);
    }

    public AcdLine newLine() {
        return (AcdLine) m_beanFactory.getBean(AcdLine.BEAN_NAME, AcdLine.class);
    }

    public AcdServer newServer() {
        return (AcdServer) m_beanFactory.getBean(AcdServer.BEAN_NAME, AcdServer.class);
    }

    public void removeServers(Collection serversIds) {
        HibernateTemplate hibernate = getHibernateTemplate();
        Collection servers = new ArrayList();
        for (Iterator i = serversIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            AcdServer server = loadServer(id);
            servers.add(server);
        }
        hibernate.deleteAll(servers);
    }

    public void removeLines(Collection linesIds) {
        HibernateTemplate hibernate = getHibernateTemplate();
        Collection servers = new HashSet();
        for (Iterator i = linesIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            AcdLine line = (AcdLine) hibernate.load(AcdLine.class, id);
            AcdServer acdServer = line.getAcdServer();
            line.associateQueue(null);
            acdServer.removeLine(line);
            servers.add(acdServer);
        }
        hibernate.saveOrUpdateAll(servers);
    }

    public AcdQueue newQueue() {
        return (AcdQueue) m_beanFactory.getBean(AcdQueue.BEAN_NAME, AcdQueue.class);
    }

    public void removeQueues(Collection queuesIds) {
        HibernateTemplate hibernate = getHibernateTemplate();
        for (Iterator i = queuesIds.iterator(); i.hasNext();) {
            Serializable id = (Serializable) i.next();
            AcdQueue queue = (AcdQueue) hibernate.load(AcdQueue.class, id);
            AcdServer acdServer = queue.getAcdServer();
            acdServer.removeQueue(queue);
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
        List agents = getHibernateTemplate().findByNamedQueryAndNamedParam("agentForUserAndServer", params, values);
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
        hibernate.save(server);
        hibernate.save(queue);
    }

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

    public Collection getAliasMappings() {
        HibernateTemplate hibernate = getHibernateTemplate();
        List acdLines = hibernate.loadAll(AcdLine.class);

        List aliases = new ArrayList();
        for (Iterator i = acdLines.iterator(); i.hasNext();) {
            AcdLine acdLine = (AcdLine) i.next();
            acdLine.appendAliases(aliases);
        }

        List<AcdServer> servers = getServers();
        for (AcdServer server : servers) {
            aliases.addAll(server.getAliasMappings());
        }

        return aliases;
    }

    public String getPresenceServiceUri() {
        List<AcdServer> servers = getServers();
        if (servers.isEmpty()) {
            return null;
        }
        // HACK: only 1 ACD server supports presence service in UI
        AcdServer server = servers.get(0);
        return server.getPresenceServiceUri();
    }

    public void moveAgentsInQueue(Serializable queueId, Collection agentsIds, int step) {
        AcdQueue queue = loadQueue(queueId);
        queue.moveAgents(agentsIds, step);
        store(queue);
    }

    public void moveQueuesInAgent(Serializable agnetId, Collection queueIds, int step) {
        AcdAgent agent = loadAgent(agnetId);
        agent.moveQueues(queueIds, step);
        store(agent);
    }

    public void clear() {
        // only need to delete servers and agents - lines and queues are handled by cascades
        List components = getHibernateTemplate().loadAll(AcdServer.class);
        components.addAll(getHibernateTemplate().loadAll(AcdAgent.class));
        getHibernateTemplate().deleteAll(components);
    }

    // trivial setters/getters below
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

    private void onLocationSpecificServiceDelete(LocationSpecificService locationService) {
        SipxService service = locationService.getSipxService();
        if (service instanceof SipxAcdService) {
            AcdServer server = getAcdServerForLocationId(locationService.getLocation().getId());
            getHibernateTemplate().delete(server);
        }
    }

    private void onLocationDelete(Location location) {
        getHibernateTemplate().update(location);
        AcdServer server = getAcdServerForLocationId(location.getId());
        if (server != null) {
            getHibernateTemplate().delete(server);
        }
    }

    public void onDelete(Object entity) {
        if (entity instanceof User) {
            onUserDelete((User) entity);
        } else if (entity instanceof LocationSpecificService) {
            onLocationSpecificServiceDelete((LocationSpecificService) entity);
        } else if (entity instanceof Location) {
            onLocationDelete((Location) entity);
        }
    }

    public void onSave(Object entity_) {
        // not interested in save events
    }

    public String getAudioServerUrl() {
        return m_audioServerUrl;
    }

    public void setAudioServerUrl(String audioServerUrl) {
        m_audioServerUrl = audioServerUrl;
    }

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

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

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

    public void addNewServer(Location location) {
        // HACK: this probably should be only called from event notification
        AcdServer acdServer = getAcdServerForLocationId(location.getId());
        if (acdServer == null) {
            acdServer = newServer();
            acdServer.setLocation(location);
            store(acdServer);
        }
    }

    public void migrateAcdServers() {
        Map<String, Integer> locationsFqdn = new HashMap<String, Integer>();
        Map<String, Integer> locationsAddress = new HashMap<String, Integer>();

        List servers = getHibernateTemplate().findByNamedQuery("allAcdServers");
        if (servers.size() > 0) {
            Location[] locations = m_locationsManager.getLocations();
            for (Location location : locations) {
                locationsFqdn.put(location.getFqdn(), location.getId());
                locationsAddress.put(location.getAddress(), location.getId());
            }
        }

        for (Iterator i = servers.iterator(); i.hasNext();) {
            Object[] row = (Object[]) i.next();
            Integer acdServerId = (Integer) row[0];
            String host = (String) row[1];
            AcdServer acdServer = getAcdServer(acdServerId);
            if (acdServer == null || acdServerId == null || host == null) {
                continue;
            }

            Location location;
            Integer locationId = locationsFqdn.get(host);
            if (locationId == null) {
                locationId = locationsAddress.get(host);
            }

            if (locationId != null) {
                // existing location
                location = m_locationsManager.getLocation(locationId);
            } else {
                // new location
                location = new Location();
                location.setFqdn(host);
                location.setName("constructed upon migration from Acd servers");
            }
            SipxAcdService acdService = (SipxAcdService) m_sipxServiceManager
                    .getServiceByBeanId(SipxAcdService.BEAN_ID);
            location.addService(acdService);
            m_locationsManager.storeLocation(location);
            getHibernateTemplate().flush();

            acdServer.setLocation(location);
            store(acdServer);
            getHibernateTemplate().flush();
        }

        cleanSchema();
    }

    private void cleanSchema() {
        try {
            Session currentSession = getHibernateTemplate().getSessionFactory().getCurrentSession();
            Connection connection = currentSession.connection();
            Statement statement = connection.createStatement();
            statement.addBatch(SQL);
            statement.executeBatch();
            statement.close();
        } catch (SQLException e) {
            LOG.warn("cleaning schema", e);
        }
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    public AcdServer getAcdServerForLocationId(Integer locationId) {
        HibernateTemplate hibernate = getHibernateTemplate();
        List<AcdServer> servers = hibernate.findByNamedQueryAndNamedParam("acdServerForLocationId", "locationId",
                locationId);

        return (AcdServer) DataAccessUtils.singleResult(servers);
    }
}
