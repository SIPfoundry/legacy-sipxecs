/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.ENABLED;
import static org.sipfoundry.commons.mongo.MongoConstants.ID;
import static org.sipfoundry.commons.mongo.MongoConstants.INTERNAL_ADDRESS;
import static org.sipfoundry.commons.mongo.MongoConstants.SERVER;
import static org.sipfoundry.commons.mongo.MongoConstants.TIMESTAMP;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;
import java.io.Writer;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.springframework.data.mongodb.core.MongoTemplate;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.tunnel.RemoteOutgoingTunnel;
import org.sipfoundry.sipxconfig.tunnel.TunnelManager;
import org.sipfoundry.sipxconfig.tunnel.TunnelProvider;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;

/**
 * This class manages all effective replications.The replication is triggered by
 * {@link ReplicationTrigger} or {@link SipxReplicationContext}, but the ReplicationManager takes
 * care of all the work load needed to replicate {@link Replicable}s in Mongo and
 * {@link ConfigurationFile}s on different locations.
 */
public class ReplicationManagerImpl extends HibernateDaoSupport implements ReplicationManager, BeanFactoryAware {
    private static final int PERMISSIONS = 0644;
    private static final Log LOG = LogFactory.getLog(ReplicationManagerImpl.class);
    private static final String REPLICATION_FAILED = "Replication: insert/update failed - ";
    private static final String REPLICATION_FAILED_REMOVE = "Replication: delete failed - ";
    private static final String LOCATION_REGISTRATION = "Location registration in db";
    private static final String DATABASE_REGENERATION = "Database regeneration";
    private static final String BRANCH_REGENERATION = "Branch regeneration";
    private static final String IP = "ip";
    private static final String DESCRIPTION = "dsc";
    private static final String MASTER = "mstr";
    private static final String SECONDS = "s | ";
    private static final String MINUTES = "m.";
    private static final String REGENERATION_OF = "Regeneration of ";
    private static final String ERROR_PERMISSION = "Error updating permission to mongo.";
    private static final String REPLICATION_INS_UPD = "Replication: inserted/updated ";
    private static final String IN = " in ";
    private static final String MS = " ms ";
    private static final DataSet[] GROUP_DATASETS = {DataSet.MAILSTORE, DataSet.PERMISSION,
        DataSet.CALLER_ALIAS, DataSet.SPEED_DIAL,
        DataSet.USER_FORWARD, DataSet.USER_LOCATION, DataSet.USER_STATIC};
    private static final DataSet[] BRANCH_DATASETS = {DataSet.USER_LOCATION};
    private static final String EXCEPTION_LOG = "IOException for stream writer";
    private static final String UTF_8 = "UTF-8";

    private boolean m_enabled = true;

    private MongoTemplate m_imdb;
    private ValidUsers m_validUsers;
    private ApiProvider<FileApi> m_fileApiProvider;
    private LocationsManager m_locationsManager;
    private AuditLogContext m_auditLogContext;
    private ListableBeanFactory m_beanFactory;
    private CoreContext m_coreContext;
    private ForwardingContext m_forwardingContext;
    private ExternalAliases m_externalAliases;
    private DataSetGenerator m_dataSetGenerator;
    private TunnelManager m_tunnelManager;
    private int m_pageSize;
    private int m_nThreads;
    private boolean m_useDynamicPageSize;

    private final Closure<User> m_userClosure = new Closure<User>() {
        @Override
        public void execute(User user) {
            replicateEntity(user);
            if (m_forwardingContext.isCallSequenceReplicable(user)) {
                CallSequence cs = m_forwardingContext.getCallSequenceForUser(user);
                replicateEntity(cs);
            }
            getHibernateTemplate().clear(); // clear the H session (see XX-9741)
        }

    };

    // the difference between the user and the group closures is that for group members
    // we need to replicate only some datasets and not all.
    // also, callsequences need not be replicated (there are no callsequnces for groups)
    private final Closure<User> m_userGroupClosure = new Closure<User>() {
        @Override
        public void execute(User user) {
            replicateEntity(user, GROUP_DATASETS);
            getHibernateTemplate().clear(); // clear the H session (see XX-9741)
        }

    };

    // the difference between the user and the group closures is that for group members
    // we need to replicate only some datasets and not all.
    // also, callsequences need not be replicated (there are no callsequnces for groups)
    private final Closure<User> m_userSpeedDialGroupClosure = new Closure<User>() {
        @Override
        public void execute(User user) {
            replicateEntity(user, DataSet.SPEED_DIAL);
            getHibernateTemplate().clear(); // clear the H session (see XX-9741)
        }

    };

    private final Closure<User> m_branchClosure = new Closure<User>() {
        @Override
        public void execute(User user) {
            replicateEntity(user, BRANCH_DATASETS);
            getHibernateTemplate().clear(); // clear the H session (see XX-9741)
        }

    };

    public void setFileApiProvider(ApiProvider<FileApi> fileApiProvider) {
        m_fileApiProvider = fileApiProvider;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Required
    public void setAuditLogContext(AuditLogContext auditLogContext) {
        m_auditLogContext = auditLogContext;
    }

    public void dropDatasetDb() {
        m_imdb.getDb().getCollection(MongoConstants.ENTITY_COLLECTION).drop();
    }

    /*
     * Callable used in the async replication of a large group of entities, namely all users. We
     * use Callable and not Runnable, b/c we need to wait for the termination of the threads
     * calling it.
     */
    private class ReplicationWorker implements Callable<Void> {
        private final int m_startIndex;
        private final int m_page;
        private final Closure<User> m_closure = m_userClosure;

        public ReplicationWorker(int index, int pageSize, Object arg) {
            m_startIndex = index;
            m_page = pageSize;
        }

        @Override
        public Void call() {
            DaoUtils.forAllUsersDo(m_coreContext, m_closure, m_startIndex, m_page);
            return null;
        }

        public int getStartIndex() {
            return m_startIndex;
        }

        public int getPage() {
            return m_page;
        }

    }

    /*
     * Callable used for the replication of members in a group
     */
    private class AllGroupMembersReplicationWorker extends ReplicationWorker {
        private final Group m_group;

        public AllGroupMembersReplicationWorker(int i, int pageSize, Group group) {
            super(i, pageSize, null);
            m_group = group;
        }

        @Override
        public Void call() {
            DaoUtils.forAllGroupMembersDo(m_coreContext, m_group, m_userGroupClosure, getStartIndex(), getPage());
            return null;
        }
    }

    /*
     * Callable used for the replication of members in a group
     */
    private class AllGroupSpeedDialMembersReplicationWorker extends ReplicationWorker {
        private final Group m_group;

        public AllGroupSpeedDialMembersReplicationWorker(int i, int pageSize, Group group) {
            super(i, pageSize, null);
            m_group = group;
        }

        @Override
        public Void call() {
            DaoUtils.forAllGroupMembersDo(m_coreContext, m_group,
                    m_userSpeedDialGroupClosure, getStartIndex(), getPage());
            return null;
        }
    }

    /*
     * Callable used for the replication of users in a branch
     */
    private class AllBranchMembersReplicationWorker extends ReplicationWorker {
        private final Branch m_branch;

        public AllBranchMembersReplicationWorker(int i, int pageSize, Branch branch) {
            super(i, pageSize, null);
            m_branch = branch;
        }

        @Override
        public Void call() {
            DaoUtils.forAllBranchMembersDo(m_coreContext, m_branch, m_branchClosure, getStartIndex(), getPage());
            return null;
        }
    }

    /*
     * Get all replicable entities and replicate them; this is far better than getting the
     * DataSet.values and generating for each of them.
     * Users are replicated using multiple threads in parallel.
     * Properties defined in sipxconfig.properties:
     * m_nThreads - number of parallel threads
     * m_pageSize - chunk of users to be processed by each thread. (Argument to sql LIMIT)
     * m_useDynamicPageSize - if set to true users will be processed in chunks of userCount/nThread
     */
    /**
     * Replicate all replicable entities. Users require special treatment, because large number of
     * users may be present which may present performance issues.
     * This method will print out replication time.
     */
    @Override
    public void replicateAllData() {
        Location primary = m_locationsManager.getPrimaryLocation();
        dropDatasetDb();
        int membersCount = m_coreContext.getAllUsersCount();
        doParallelAsyncReplication(membersCount, ReplicationWorker.class, null);
        // get the rest of Replicables and replicate them
        Map<String, ReplicableProvider> beanMap = m_beanFactory.getBeansOfType(ReplicableProvider.class);
        for (ReplicableProvider provider : beanMap.values()) {
            for (Replicable entity : provider.getReplicables()) {
                replicateEntity(entity);
            }
        }
        // Replicate the external aliases
        ExternalAlias extalias = new ExternalAlias();
        extalias.setFiles(m_externalAliases.getFiles());
        replicateEntity(extalias);
        m_auditLogContext.logReplicationMongo(DATABASE_REGENERATION, primary);
    }

    /**
     * Replicates a single entity. It retrieves the {@link DataSet}s defined in {@link
     * Replicable.getDataSets()} and generates the datasets for the entity.
     */
    @Override
    public void replicateEntity(Replicable entity) {
        String name = (entity.getName() != null) ? entity.getName() : entity.toString();
        try {
            Long start = System.currentTimeMillis();
            DBObject top = m_dataSetGenerator.findOrCreate(entity);
            Set<DataSet> dataSets = entity.getDataSets();
            for (DataSet dataSet : dataSets) {
                replicateEntity(entity, dataSet, top);
            }
            Long end = System.currentTimeMillis();
            LOG.debug(REPLICATION_INS_UPD + name + IN + (end - start) + MS);
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED + name, e);
            throw new UserException(REPLICATION_FAILED + entity.getName(), e);
        }
    }

    /**
     * Replicates an array of {@link DataSet}s for a given {@link Replicable}
     */
    @Override
    public void replicateEntity(Replicable entity, DataSet... dataSet) {
        String name = (entity.getName() != null) ? entity.getName() : entity.toString();
        try {
            Long start = System.currentTimeMillis();
            DBObject top = m_dataSetGenerator.findOrCreate(entity);
            for (int i = 0; i < dataSet.length; i++) {
                replicateEntity(entity, dataSet[i], top);
            }
            Long end = System.currentTimeMillis();
            LOG.debug(REPLICATION_INS_UPD + name + IN + (end - start) + MS);
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED + name, e);
            throw new UserException(REPLICATION_FAILED + entity.getName(), e);
        }
    }

    private void replicateEntity(Replicable entity, DataSet dataSet, DBObject top) {
        String beanName = dataSet.getBeanName();
        final DataSetGenerator generator = m_beanFactory.getBean(beanName, DataSetGenerator.class);
        generator.generate(entity, top);
        LOG.debug("Entity " + entity.getName() + " updated.");
    }

    /**
     * Replicate only a specified DataSet for all entities.
     */
    // TODO: figure out if we need to use parallel processing.
    @Override
    public void replicateAllData(final DataSet ds) {
        try {
            Long start = System.currentTimeMillis();
            Map<String, ReplicableProvider> beanMap = m_beanFactory.getBeansOfType(ReplicableProvider.class);
            for (ReplicableProvider provider : beanMap.values()) {
                for (Replicable entity : provider.getReplicables()) {
                    if (!entity.getDataSets().contains(ds)) {
                        continue;
                    }
                    DBObject top = m_dataSetGenerator.findOrCreate(entity);
                    replicateEntity(entity, ds, top);
                }
            }
            Closure<User> closure = new Closure<User>() {
                @Override
                public void execute(User user) {
                    DBObject top = m_dataSetGenerator.findOrCreate(user);
                    replicateEntity(user, ds, top);
                }
            };
            DaoUtils.forAllUsersDo(m_coreContext, closure);
            Long end = System.currentTimeMillis();
            LOG.info(REGENERATION_OF + ds.getName() + " completed in " + (end - start) / 1000 + SECONDS
                    + (end - start) / 1000 / 60 + MINUTES);

        } catch (Exception e) {
            LOG.error(REGENERATION_OF + ds.getName() + " failed", e);
            throw new UserException(e);
        }
    }


    @Override
    public void replicateGroup(Group group) {
        replicateGroupWithWorker(group, AllGroupMembersReplicationWorker.class);
    }

    @Override
    public void replicateSpeedDialGroup(Group group) {
        replicateGroupWithWorker(group, AllGroupSpeedDialMembersReplicationWorker.class);
    }

    private void replicateGroupWithWorker(Group group, Class<? extends ReplicationWorker> worker) {
        Location primary = m_locationsManager.getPrimaryLocation();
        try {
            int membersCount = m_coreContext.getGroupMembersCount(group.getId());
            doParallelAsyncReplication(membersCount, worker, group);
            LOG.info("Regeneration of group complete");
        } catch (Exception e) {
            LOG.error("Regeneration of group failed", e);
            throw new UserException(e);
        }
    }


    @Override
    public void replicateBranch(Branch branch) {
        Location primary = m_locationsManager.getPrimaryLocation();
        try {
            int membersCount = m_coreContext.getBranchMembersCount(branch.getId());
            doParallelAsyncReplication(membersCount, AllBranchMembersReplicationWorker.class, branch);
            m_auditLogContext.logReplicationMongo(BRANCH_REGENERATION, primary);
        } catch (Exception e) {
            m_auditLogContext.logReplicationMongoFailed(BRANCH_REGENERATION, primary, e);
            LOG.error("Regeneration of branch failed", e);
            throw new UserException(e);
        }
    }


    @Override
    public void deleteBranch(Branch branch) {
        try {
            LOG.info("Starting regeneration of branch members.");
            DBCursor users = m_validUsers.getUsersInBranch(branch.getName());
            for (DBObject user : users) {
                String uid = user.get(MongoConstants.UID).toString();
                User u = m_coreContext.loadUserByUserName(uid);
                replicateEntity(u, BRANCH_DATASETS);
                getHibernateTemplate().clear(); // clear the H session (see XX-9741)
            }
            LOG.info("End of regeneration of branch members.");
        } catch (Exception e) {
            LOG.error(ERROR_PERMISSION, e);
            throw new UserException(ERROR_PERMISSION, e);
        }
    }

    @Override
    public void deleteGroup(Group group) {
        try {
            LOG.info("Starting regeneration of group members.");
            DBCursor users = m_validUsers.getUsersInGroup(group.getName());
            for (DBObject user : users) {
                String uid = user.get(MongoConstants.UID).toString();
                User u = m_coreContext.loadUserByUserName(uid);
                replicateEntity(u, GROUP_DATASETS);
                getHibernateTemplate().clear(); // clear the H session (see XX-9741)
            }
            LOG.info("End of regeneration of group members.");
        } catch (Exception e) {
            LOG.error(ERROR_PERMISSION, e);
            throw new UserException(ERROR_PERMISSION, e);
        }
    }

    /*
     * synchronise here. We do not want multiple threads doing heavy replication stuff at the same time.
     * (i.e. if we hit send profiles, then do a change on group with 20.000 members)
     */
    private synchronized void doParallelAsyncReplication(int membersCount,
            Class<? extends ReplicationWorker> cls, Object type) {
        ExecutorService replicationExecutorService = Executors.newFixedThreadPool(m_nThreads);
        Long start = System.currentTimeMillis();
        int pageSize = m_pageSize;
        if (m_useDynamicPageSize) {
            pageSize = membersCount / m_nThreads + 1;
        }
        int pages = new Double(Math.ceil(membersCount
                / pageSize)).intValue() + 1;
        Constructor<? extends ReplicationWorker> ct = (Constructor< ? extends ReplicationWorker>)
            cls.getConstructors()[0];
        List<Future<Void>> futures = new ArrayList<Future<Void>>();
        LOG.info("Starting async parallel regeneration of mongo group of "
                + membersCount + " entities on " + m_nThreads
                + " threads using chunks of " + pageSize
                + " users");
        for (int i = 0; i < pages; i++) {
            ReplicationWorker worker = null;
            try {
                worker = ct.newInstance(this, i * pageSize, pageSize, type);
            } catch (InstantiationException e) {
                throw new RuntimeException(e);
            } catch (IllegalAccessException e) {
                throw new RuntimeException(e);
            } catch (InvocationTargetException e) {
                throw new RuntimeException(e);
            }
            futures.add(replicationExecutorService.submit(worker));
        }
        for (Future<Void> future : futures) {
            try {
                future.get();
            } catch (InterruptedException e) {
                throw new RuntimeException(e);
            } catch (ExecutionException e) {
                throw new RuntimeException(e);
            }
        }
        replicationExecutorService.shutdown();
        Long end = System.currentTimeMillis();
        LOG.info("Regeneration of entities finished in " + (end - start) / 1000 + SECONDS + (end - start) / 1000
                / 60 + MINUTES);
    }

    /**
     * Replicate a Location. That means just register a location in Mongo node DB
     * and register the tunnels.
     */
    @Override
    public void replicateLocation(Location location) {
        if (location.isRegistered()) {
            DBCollection nodeCollection = m_imdb.getDb().getCollection(MongoConstants.NODE_COLLECTION);
            DBObject search = new BasicDBObject();
            search.put(ID, location.getId());
            DBCursor cursor = nodeCollection.find(search);
            DBObject node = new BasicDBObject();
            if (cursor.hasNext()) {
                node = cursor.next();
            }
            node.put(ID, location.getId());
            node.put(IP, location.getAddress());
            node.put(DESCRIPTION, location.getName());
            node.put(MASTER, location.isPrimary());
            nodeCollection.save(node);
        }
        registerTunnels(location);
        m_auditLogContext.logReplicationMongo(LOCATION_REGISTRATION, location);
    }

    @Override
    public void registerTunnels(Location location) {
        DBCollection registrarNode = m_imdb.getDb().getCollection(MongoConstants.STUNNEL_COLLECTION);
        registrarNode.drop();

        Location[] locations = m_locationsManager.getLocations();
        if (locations.length > 1) {
            for (int i = 0; i < locations.length; i++) {
                List<Location> otherLocations = new ArrayList<Location>(Arrays.asList(locations));
                otherLocations.remove(locations[i]);

                for (TunnelProvider tunnelProvider : m_tunnelManager.getTunnelProviders()) {
                    List<RemoteOutgoingTunnel> tunnels = (List<RemoteOutgoingTunnel>) tunnelProvider
                    .getClientSideTunnels(otherLocations, location);
                    for (RemoteOutgoingTunnel tunnel : tunnels) {
                        DBObject search = new BasicDBObject();
                        search.put(ID, tunnel.getName());
                        DBObject server = registrarNode.findOne(search);
                        if (server == null) {
                            server = new BasicDBObject();
                            server.put(ID, tunnel.getName());
                        }
                        server.put(SERVER, "localhost:" + tunnel.getLocalhostPort());
                        server.put(INTERNAL_ADDRESS, tunnel.getRemoteMachineAddress());
                        Location loc = m_locationsManager.getLocationByAddress(tunnel.getRemoteMachineAddress());
                        server.put(ENABLED, loc.isRegistered());
                        registrarNode.save(server);
                    }
                }
            }
            DBObject timestamp = new BasicDBObject();
            timestamp.put(ID, "timestamp");
            timestamp.put(TIMESTAMP, new Long(System.currentTimeMillis() / 1000).toString());
            registrarNode.save(timestamp);
        }
    }

    /**
     * Removes an entity from Mongo imdb
     */
    @Override
    public void removeEntity(Replicable entity) {
        try {
            String id = DataSetGenerator.getEntityId(entity);
            remove(MongoConstants.ENTITY_COLLECTION, id);
            LOG.info("Replication: removed " + entity.getName());
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED_REMOVE + entity.getName(), e);
            throw new UserException(REPLICATION_FAILED_REMOVE + entity.getName(), e);
        }
    }

    /**
     * Unregisters Location from node db.
     */
    @Override
    public void removeLocation(Location location) {
        try {
            if (location.isRegistered()) {
                remove(MongoConstants.NODE_COLLECTION, location.getId());
            }
            registerTunnels(location);
        } catch (Exception e) {
            throw new UserException("Cannot unregister location in mongo db: " + e);
        }
    }

    private DBCollection getEntityCollection() {
        return m_imdb.getDb().getCollection(MongoConstants.ENTITY_COLLECTION);
    }

    /**
     * shortcut to remove objects from mongo's imdb database
     */
    private void remove(String collectionName, Object id) {
        DBCollection collection = m_imdb.getDb().getCollection(collectionName);
        DBObject search = new BasicDBObject();
        search.put(ID, id);
        DBObject node = collection.findOne(search);
        //necessary only in case of CallSequences
        //(user delete will trigger CS delete but CS for user may not exist)
        if (node != null) {
            collection.remove(node);
        }
    }

    /**
     * This method will issue a sync command to the selected secondary (i.e. mongo slave) THis is
     * used in order to force the slave to pickup latest changes. It will do so if and only if the
     * slave is "stale". Check out Mongo documentation for more info.
     */
    // TODO: figure out another method to resync slaves, maybe issue a supervisor command
    // to delete mongo dbs to force a full replication on the slave
    @Override
    public void resyncSlave(Location location) {
//  !!!!!!!!!!!!  NOTE: This cannot work, it needs the stunnel port numbers!
//
//       try {
//            ResyncResult result = MongoAccessController.INSTANCE.resync(location.getAddress());
//            if (result.getStatus() == 0) {
//                LOG.error("Replication: resync - " + result.getErrorMessage());
//            } else {
//                LOG.info("Replication: resync started - " + location.getAddress());
//            }
//
//        } catch (Exception e) {
//            LOG.error("Replication: resync failed - " + location.getAddress(), e);
//        }
    }

    /**
     * Encodes payload using Base64 and returns encoded data as string
     *
     * @param payload
     * @return string representing encoded data
     */
    private String encodeBase64(byte[] payload) {
        try {
            // Base64 encoded content is always limited to US-ASCII charset
            byte[] encodedPayload = Base64.encodeBase64(payload);
            return new String(encodedPayload, "US-ASCII");
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Adds the specified Permission to all entities supporting permissions. Used only when a new
     * permission with default "checked" is added. Much faster than using
     * replicateAllData(DataSet.PERMISSION)
     */
    @Override
    public void addPermission(Permission permission) {
        try {
            DBCursor users = m_validUsers.getEntitiesWithPermissions();
            for (DBObject user : users) {
                Collection<String> prms = (Collection<String>) user.get(MongoConstants.PERMISSIONS);
                prms.add(permission.getName());
                user.put(MongoConstants.PERMISSIONS, prms);
                getEntityCollection().save(user);
            }
        } catch (Exception e) {
            LOG.error(ERROR_PERMISSION, e);
            throw new UserException(ERROR_PERMISSION, e);
        }
    }

    /**
     * Removes the specified Permission from the entities that have it.
     */
    @Override
    public void removePermission(Permission permission) {
        try {
            DBCursor users = m_validUsers.getEntitiesWithPermission(permission.getName());
            for (DBObject user : users) {
                Collection<String> prms = (Collection<String>) user.get(MongoConstants.PERMISSIONS);
                prms.remove(permission.getName());
                user.put(MongoConstants.PERMISSIONS, prms);
                getEntityCollection().save(user);
            }
        } catch (Exception e) {
            LOG.error(ERROR_PERMISSION, e);
            throw new UserException(ERROR_PERMISSION, e);
        }
    }

    /**
     * Replicates file on all locations
     */
    @Override
    public void replicateFile(Location location, ConfigurationFile file) {
        Location p = m_locationsManager.getPrimaryLocation();
        if (p != null) {
            replicateFile(p.getFqdn(), location, file);
        }
    }

    void replicateFile(String primaryHostname, Location location, ConfigurationFile file) {
        if (!m_enabled) {
            return;
        }

        ByteArrayOutputStream outStream = new ByteArrayOutputStream();

        if (!location.isRegistered() || !location.isReplicateConfig()) {
            return;
        }
        if (!file.isReplicable(location)) {
            LOG.info("File " + file.getName() + " cannot be replicated on location: " + location.getFqdn());
            return;
        }
        boolean success = false;
        try {
            outStream = new ByteArrayOutputStream();
            LOG.debug("Generate location dependent content for " + file.getName()
                   + " on location " + location.getFqdn());
            Writer writer = new OutputStreamWriter(outStream, UTF_8);
            file.write(writer, location);
            writer.close();
            byte[] payloadBytes = outStream.toByteArray();
            String content = encodeBase64(payloadBytes);

            FileApi api = m_fileApiProvider.getApi(location.getProcessMonitorUrl());
            success = api.replace(primaryHostname, file.getPath(), PERMISSIONS, content);
        } catch (XmlRpcRemoteException e) {
            LOG.error("File replication failed: " + file.getName() + "; on " + location.getFqdn(), e);
        } catch (UnsupportedEncodingException e) {
            LOG.error("UTF-8 encoding should be always supported.");
            throw new RuntimeException(e);
        } catch (IOException e) {
            LOG.error(EXCEPTION_LOG, e);
            throw new RuntimeException(e);
        } finally {
            if (success) {
                m_auditLogContext.logReplication(file.getName(), location);
            } else {
                m_auditLogContext.logReplicationFailed(file.getName(), location, null);
            }
        }
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void setExternalAliases(ExternalAliases externalAliases) {
        m_externalAliases = externalAliases;
    }

    public void setDataSetGenerator(DataSetGenerator dataSetGenerator) {
        m_dataSetGenerator = dataSetGenerator;
    }

    public void setPageSize(int pageSize) {
        m_pageSize = pageSize;
    }

    public void setUseDynamicPageSize(boolean useDynamicPageSize) {
        m_useDynamicPageSize = useDynamicPageSize;
    }

    public void setnThreads(int nThreads) {
        m_nThreads = nThreads;
    }

    public void setTunnelManager(TunnelManager tunnelManager) {
        m_tunnelManager = tunnelManager;
    }

    public ValidUsers getValidUsers() {
        return m_validUsers;
    }

    public void setValidUsers(ValidUsers validUsers) {
        m_validUsers = validUsers;
    }

    public MongoTemplate getImdb() {
        return m_imdb;
    }

    public void setImdb(MongoTemplate imdb) {
        m_imdb = imdb;
    }
}
