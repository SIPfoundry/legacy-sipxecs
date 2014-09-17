/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.ENTITY_NAME;
import static org.sipfoundry.commons.mongo.MongoConstants.GROUPS;
import static org.sipfoundry.commons.mongo.MongoConstants.ID;
import static org.sipfoundry.commons.mongo.MongoConstants.IDENTITY;
import static org.sipfoundry.commons.mongo.MongoConstants.VALID_USER;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Proxy;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.MongoGenerationFinishedEvent;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.SpecialUser;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.VersionInfo;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.logging.AuditLogContext;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setup.SetupListener;
import org.sipfoundry.sipxconfig.setup.SetupManager;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.NoSuchBeanDefinitionException;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.data.mongodb.core.MongoTemplate;

import com.mongodb.BasicDBObject;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.MongoException;
import com.mongodb.QueryBuilder;

/**
 * This class manages all effective replications.The replication is triggered by
 * {@link ReplicationTrigger} or SipxReplicationContext, but the ReplicationManager takes care of
 * all the work load needed to replicate {@link Replicable}s in Mongo and
 * {@link ConfigurationFile}s on different locations.
 */
public class ReplicationManagerImpl extends SipxHibernateDaoSupport implements ReplicationManager, BeanFactoryAware,
        SetupListener, ApplicationContextAware {
    private static final Log LOG = LogFactory.getLog(ReplicationManagerImpl.class);
    private static final String REPLICATION_FAILED = "Replication: insert/update failed - ";
    private static final String REPLICATION_FAILED_REMOVE = "Replication: delete failed - ";
    private static final String DATABASE_REGENERATION = "Database regeneration";
    private static final String BRANCH_REGENERATION = "Branch regeneration";
    private static final String SECONDS = "s | ";
    private static final String MINUTES = "m.";
    private static final String REGENERATION_OF = "Regeneration of ";
    private static final String ERROR_PERMISSION = "Error updating permission to mongo.";
    private static final String REPLICATION_INS_UPD = "Replication: inserted/updated ";
    private static final String IN = " in ";
    private static final String MS = " ms ";
    private static final DataSet[] GROUP_DATASETS = {
        DataSet.ATTENDANT, DataSet.PERMISSION, DataSet.CALLER_ALIAS, DataSet.SPEED_DIAL, DataSet.USER_FORWARD,
        DataSet.USER_LOCATION, DataSet.USER_STATIC, DataSet.MAILSTORE, DataSet.E911
    };
    private static final DataSet[] PHONE_GROUP_DATASETS = {
        DataSet.E911
    };
    private static final DataSet[] BRANCH_DATASETS = {
        DataSet.USER_LOCATION
    };
    private MongoTemplate m_imdb;
    private ValidUsers m_validUsers;
    private LocationsManager m_locationsManager;
    private AuditLogContext m_auditLogContext;
    private ListableBeanFactory m_beanFactory;
    private CoreContext m_coreContext;
    private ExternalAliases m_externalAliases;
    private int m_pageSize = 1000;
    private int m_nThreads = 2;
    private boolean m_useDynamicPageSize;
    private DataSet m_dataSet;
    private ApplicationContext m_applicationContext;
    private PhoneContext m_phoneContext;

    private final Closure<User> m_userClosure = new Closure<User>() {
        @Override
        public void execute(User user) {
            replicateEntity(user);
            getHibernateTemplate().clear(); // clear the H session (see XX-9741)
        }
    };

    private final Closure<User> m_userClosureDataSet = new Closure<User>() {

        @Override
        public void execute(User user) {
            replicateEntity(user, m_dataSet);
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

    private final Closure<User> m_branchClosure = new Closure<User>() {
        @Override
        public void execute(User user) {
            replicateEntity(user, BRANCH_DATASETS);
            getHibernateTemplate().clear(); // clear the H session (see XX-9741)
        }
    };

    private final Closure<Phone> m_phoneClosure = new Closure<Phone>() {
        @Override
        public void execute(Phone phone) {
            replicateEntity(phone);
            getHibernateTemplate().clear(); // clear the H session (see XX-9741)
        }
    };

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
     * Callable used in the parallel replication of a large group of entities, namely all users.
     * We use Callable and not Runnable, b/c we need to wait for the termination of the threads
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

    private class ReplicationWorkerDataSet extends ReplicationWorker {

        public ReplicationWorkerDataSet(int index, int pageSize, Object arg) {
            super(index, pageSize, arg);
        }

        @Override
        public Void call() {
            DaoUtils.forAllUsersDo(m_coreContext, m_userClosureDataSet, getStartIndex(), getPage());
            return null;
        }
    }

    private class ReplicationWorkerPhone extends ReplicationWorker {

        public ReplicationWorkerPhone(int index, int pageSize, Object arg) {
            super(index, pageSize, arg);
        }

        @Override
        public Void call() {
            DaoUtils.forAllPhonesDo(m_phoneContext, m_phoneClosure, getStartIndex(), getPage());
            return null;
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

    private class AllPhoneGroupMembersReplicationWorker extends ReplicationWorker {
        private final Group m_group;

        public AllPhoneGroupMembersReplicationWorker(int i, int pageSize, Group group) {
            super(i, pageSize, null);
            m_group = group;
        }

        @Override
        public Void call() {
            DaoUtils.forAllPhoneGroupMembersDo(m_phoneContext, m_group, m_phoneClosure, getStartIndex(), getPage());
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
     * DataSet.values and generating for each of them. Users are replicated using multiple threads
     * in parallel. Properties defined in sipxconfig.properties: m_nThreads - number of parallel
     * threads m_pageSize - chunk of users to be processed by each thread. (Argument to sql LIMIT)
     * m_useDynamicPageSize - if set to true users will be processed in chunks of
     * userCount/nThread
     */
    /**
     * Replicate all replicable entities. Users require special treatment, because large number of
     * users may be present which may present performance issues. This method will print out
     * replication time.
     */
    @Override
    public void replicateAllData() {
        try {
            Location primary = m_locationsManager.getPrimaryLocation();
            dropDatasetDb();
            int membersCount = m_coreContext.getAllUsersCount();
            doParallelReplication(membersCount, ReplicationWorker.class, null);
            int phonesCount = m_phoneContext.getPhonesCount();
            doParallelReplication(phonesCount, ReplicationWorkerPhone.class, null);
            // get the rest of Replicables and replicate them
            Map<String, ReplicableProvider> beanMap = m_beanFactory.getBeansOfType(ReplicableProvider.class);
            for (ReplicableProvider provider : beanMap.values()) {
                if (provider instanceof Proxy) {
                    for (Replicable entity : provider.getReplicables()) {
                        replicateEntity(entity);
                    }
                }
            }
            // Replicate the external aliases
            ExternalAlias extalias = new ExternalAlias();
            extalias.setFiles(m_externalAliases.getFiles());
            replicateEntity(extalias);
            m_auditLogContext.logReplicationMongo(DATABASE_REGENERATION, primary);
        } finally {
            m_applicationContext.publishEvent(new MongoGenerationFinishedEvent(this));
        }

    }

    /**
     * Replicates a single entity. It retrieves the {@link DataSet}s defined in {@link
     * Replicable.getDataSets()} and generates the datasets for the entity.
     */
    @Override
    public void replicateEntity(Replicable entity) {
        if (!entity.isReplicationEnabled()) {
            removeEntity(entity);
            return;
        }
        String name = (entity.getName() != null) ? entity.getName() : entity.toString();
        try {
            Set<DataSet> dataSets = entity.getDataSets();
            if (dataSets != null && !dataSets.isEmpty()) {
                replicateEntity(entity, dataSets.toArray(new DataSet[dataSets.size()]));
            } else {
                DBObject top = new BasicDBObject();
                DBObject cleanCopy = new BasicDBObject();
                boolean isNew = findOrCreate(entity, top, cleanCopy);
                replicate(top, cleanCopy, name, isNew);
            }
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED + name, e);
            throw new UserException(REPLICATION_FAILED + entity.getName(), e);
        }
    }

    /**
     * Replicates an array of {@link DataSet}s for a given {@link Replicable}
     */
    @Override
    public void replicateEntity(Replicable entity, DataSet... dataSets) {
        if (!entity.isReplicationEnabled()) {
            removeEntity(entity);
            return;
        }
        String name = (entity.getName() != null) ? entity.getName() : entity.toString();
        try {
            Long start = System.currentTimeMillis();
            DBObject top = new BasicDBObject();
            DBObject cleanCopy = new BasicDBObject();
            boolean isNew = findOrCreate(entity, top, cleanCopy);
            for (DataSet dataSet : dataSets) {
                replicateEntity(entity, dataSet, top);
            }
            replicate(top, cleanCopy, name, isNew);
            Long end = System.currentTimeMillis();
            LOG.debug(REPLICATION_INS_UPD + name + IN + (end - start) + MS);
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED + name, e);
            throw new UserException(REPLICATION_FAILED + entity.getName(), e);
        }
    }

    private void replicate(DBObject top, DBObject cleanCopy, String name, boolean isNew) {
        if (isNew) {
            getDbCollection().save(top);
        } else {
            DBObject toUpdate = new BasicDBObject();
            toUpdate.put(ID, top.get(ID));
            DBObject updateQ = new BasicDBObject();
            DBObject removeQ = new BasicDBObject();
            for (String field : cleanCopy.keySet()) {
                Object oldValue = cleanCopy.get(field);
                Object newValue = top.get(field);
                LOG.debug(String.format("field: %s;old: %s; new: %s", field, oldValue, newValue));
                if (oldValue == null || !oldValue.equals(newValue)) {
                    if (newValue == null) {
                        removeQ.put(field, StringUtils.EMPTY);
                    } else {
                        updateQ.put(field, newValue);
                    }
                }
            }
            for (String field : top.keySet()) {
                Object oldValue = cleanCopy.get(field);
                Object newValue = top.get(field);

                if (newValue != null && (oldValue == null || !oldValue.equals(newValue))) {
                    updateQ.put(field, newValue);
                }
            }
            LOG.debug(String.format("Update query: %s: ", updateQ));
            LOG.debug(String.format("Remove query: %s: ", removeQ));
            DBObject set = new BasicDBObject("$set", updateQ).append("$unset", removeQ);
            getDbCollection().update(toUpdate, set);
        }

    }

    private void replicateEntity(Replicable entity, DataSet dataSet, DBObject top) {
        String beanName = dataSet.getBeanName();
        try {
            final AbstractDataSetGenerator generator = m_beanFactory.getBean(beanName,
                    AbstractDataSetGenerator.class);
            generator.generate(entity, top);
        } catch (NoSuchBeanDefinitionException e) {
            // This will happen always for datasets defined in plugins.
            // Logging will only litter the logs.
            // LOG.debug("No such bean: " + beanName);
            return;
        }
    }

    /**
     * Replicate only a specified DataSet for all entities.
     */
    @Override
    public void replicateAllData(final DataSet ds) {
        try {
            long start = System.currentTimeMillis();
            Map<String, ReplicableProvider> beanMap = m_beanFactory.getBeansOfType(ReplicableProvider.class);
            for (ReplicableProvider provider : beanMap.values()) {
                for (Replicable entity : provider.getReplicables()) {
                    if (entity != null) {
                        if (!entity.getDataSets().contains(ds)) { /*
                                                                   * Callable used for the
                                                                   * replication of members in a
                                                                   * group
                                                                   */

                            continue;
                        }
                        replicateEntity(entity);
                    }
                }
            }
            int membersCount = m_coreContext.getAllUsersCount();
            m_dataSet = ds;
            doParallelReplication(membersCount, ReplicationWorkerDataSet.class, null);
            long end = System.currentTimeMillis();
            LOG.info(REGENERATION_OF + ds.getName() + " completed in " + (end - start) / 1000 + SECONDS
                    + (end - start) / 1000 / 60 + MINUTES);

        } catch (Exception e) {
            LOG.error(REGENERATION_OF + ds.getName() + " failed", e);
            throw new UserException(e);
        }
    }

    @Override
    /*
     * Callable used for the replication of members in a group
     */
    public void replicateGroup(Group group) {
        int membersCount = m_coreContext.getGroupMembersCount(group.getId());
        LOG.debug("Replicate user group " + group.getName() + " with " + membersCount + " members");
        replicateGroupWithWorker(group, AllGroupMembersReplicationWorker.class, membersCount);
    }

    @Override
    public void replicatePhoneGroup(Group group) {
        int membersCount = m_phoneContext.getPhonesInGroupCount(group.getId());
        replicateGroupWithWorker(group, AllPhoneGroupMembersReplicationWorker.class, membersCount);
    }

    private void replicateGroupWithWorker(Group group, Class< ? extends ReplicationWorker> worker, int membersCount) {
        try {
            doParallelReplication(membersCount, worker, group);
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
            doParallelReplication(membersCount, AllBranchMembersReplicationWorker.class, branch);
            m_auditLogContext.logReplicationMongo(BRANCH_REGENERATION, primary);
        } catch (Exception e) {
            m_auditLogContext.logReplicationMongoFailed(BRANCH_REGENERATION, primary, e);
            LOG.error("Regeneration of branch failed", e);
            throw new UserException(e);
        }
    }

    @Override
    public void deleteBranch(Branch branch) {
        LOG.info("Starting regeneration of branch members.");
        DBCursor users = m_validUsers.getUsersInBranch(branch.getName());
        try {
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
        } finally {
            users.close();
        }
    }

    @Override
    public void deleteGroup(Group group) {
        LOG.info("Starting regeneration of group members.");
        if (User.GROUP_RESOURCE_ID.equals(group.getResource())) {
            DBCursor users = m_validUsers.getUsersInGroup(group.getName());
            try {
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
            } finally {
                users.close();
            }

        } else if (Phone.GROUP_RESOURCE_ID.equals(group.getResource())) {
            DBObject query = QueryBuilder.start(ENTITY_NAME).is("phone").and(GROUPS).is(group.getName()).get();
            DBCursor phones = getEntityCollection().find(query);
            try {
                for (DBObject phone : phones) {
                    String serialNumber = phone.get(MongoConstants.SERIAL_NUMBER).toString();
                    Phone p = m_phoneContext.getPhoneBySerialNumber(serialNumber);
                    replicateEntity(p, PHONE_GROUP_DATASETS);
                    getHibernateTemplate().clear(); // clear the H session (see XX-9741)
                }
            } catch (Exception e) {
                LOG.error(e);
                throw new UserException(e);
            } finally {
                phones.close();
            }
        }

    }

    /*
     * synchronise here. We do not want multiple threads doing heavy replication stuff at the same
     * time. (i.e. if we hit send profiles, then do a change on group with 20.000 members)
     */
    private synchronized void doParallelReplication(int membersCount, Class< ? extends ReplicationWorker> cls,
            Object type) {
        ExecutorService replicationExecutorService = Executors.newFixedThreadPool(m_nThreads);
        Long start = System.currentTimeMillis();
        int pageSize = m_pageSize;
        if (m_useDynamicPageSize) {
            pageSize = membersCount / m_nThreads + 1;
        }
        int pages = new Double(Math.ceil(membersCount / pageSize)).intValue() + 1;
        Constructor< ? extends ReplicationWorker> ct = (Constructor< ? extends ReplicationWorker>) cls
                .getConstructors()[0];
        List<Future<Void>> futures = new ArrayList<Future<Void>>();
        LOG.info("Starting parallel regeneration of mongo group of " + membersCount + " entities on " + m_nThreads
                + " threads using chunks of " + pageSize + " users");
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
     * Removes an entity from Mongo imdb
     */
    @Override
    public void removeEntity(Replicable entity) {
        try {
            String id = getEntityId(entity);
            remove(MongoConstants.ENTITY_COLLECTION, id);
            LOG.info("Replication: removed " + entity.getName());
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED_REMOVE + entity.getName(), e);
            throw new UserException(REPLICATION_FAILED_REMOVE + entity.getName(), e);
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
        // necessary only in case of CallSequences
        // (user delete will trigger CS delete but CS for user may not exist)
        if (node != null) {
            collection.remove(node);
        }
    }

    /**
     * Adds the specified Permission to all entities supporting permissions. Used only when a new
     * permission with default "checked" is added. Much faster than using
     * replicateAllData(DataSet.PERMISSION)
     */
    @Override
    public void addPermission(Permission permission) {
        DBCursor users = m_validUsers.getEntitiesWithPermissions();
        try {
            for (DBObject user : users) {
                Collection<String> prms = (Collection<String>) user.get(MongoConstants.PERMISSIONS);
                prms.add(permission.getName());
                user.put(MongoConstants.PERMISSIONS, prms);
                getEntityCollection().save(user);
            }
        } catch (Exception e) {
            LOG.error(ERROR_PERMISSION, e);
            throw new UserException(ERROR_PERMISSION, e);
        } finally {
            users.close();
        }
    }

    /**
     * Removes the specified Permission from the entities that have it.
     */
    @Override
    public void removePermission(Permission permission) {
        DBCursor users = m_validUsers.getEntitiesWithPermission(permission.getName());
        try {
            for (DBObject user : users) {
                Collection<String> prms = (Collection<String>) user.get(MongoConstants.PERMISSIONS);
                prms.remove(permission.getName());
                user.put(MongoConstants.PERMISSIONS, prms);
                getEntityCollection().save(user);
            }
        } catch (Exception e) {
            LOG.error(ERROR_PERMISSION, e);
            throw new UserException(ERROR_PERMISSION, e);
        } finally {
            users.close();
        }
    }

    /**
     * This method searches for an entity in Mongo. If it cannot find it creates one with
     * necessary properties. It receives the Replicable entity as a parameter and a new
     * BasicDbObject. It returns a boolean value if the entity was created (true) or found
     * (false).
     *
     * @param entity
     * @param obj
     * @return
     */
    protected boolean findOrCreate(Replicable entity, DBObject obj, DBObject cleanCopy) {
        DBCollection collection = getDbCollection();
        String id = getEntityId(entity);
        boolean isNew = false;
        DBObject search = new BasicDBObject();
        search.put(ID, id);
        DBObject top = collection.findOne(search);
        if (top == null) {
            isNew = true;
            top = new BasicDBObject();
            top.put(ID, id);
        }
        cleanCopy.putAll(top.toMap());
        String sipDomain = m_coreContext.getDomainName();
        if (entity.getIdentity(sipDomain) != null) {
            top.put(IDENTITY, entity.getIdentity(sipDomain));
        }
        Map<String, Object> mongoProps = entity.getMongoProperties(sipDomain);

        for (Map.Entry<String, Object> property : mongoProps.entrySet()) {
            top.put(property.getKey(), property.getValue());
        }
        if (entity.isValidUser()) {
            top.put(VALID_USER, true);
        }
        top.put(ENTITY_NAME, entity.getEntityName().toLowerCase());
        obj.putAll(top.toMap());
        return isNew;
    }

    private static String getEntityId(Replicable entity) {
        String id = "";
        if (entity instanceof BeanWithId) {
            id = entity.getEntityName() + ((BeanWithId) entity).getId();
        }
        if (entity instanceof SpecialUser) {
            id = ((SpecialUser) entity).getUserName();
        } else if (entity instanceof User) {
            User u = (User) entity;
            if (u.isNew()) {
                id = u.getUserName();
            }
        } else if (entity instanceof ExternalAlias) {
            ExternalAlias alias = (ExternalAlias) entity;
            id = alias.getName();
        }
        return id;
    }

    @Override
    public boolean testDatabaseReady() {
        try {
            getDbCollection();
            return true;
        } catch (MongoException e) {
            return false;
        }
    }

    public DBCollection getDbCollection() {
        DBCollection entity = m_imdb.getDb().getCollection(MongoConstants.ENTITY_COLLECTION);
        DBObject index1 = new BasicDBObject();
        index1.put(MongoConstants.ALIASES + "." + MongoConstants.ALIAS_ID, 1);
        DBObject index2 = new BasicDBObject();
        index2.put(MongoConstants.UID, 1);
        DBObject index3 = new BasicDBObject();
        index3.put(MongoConstants.IDENTITY, 1);
        DBObject index4 = new BasicDBObject();
        index4.put(MongoConstants.GROUPS, 1);
        DBObject index5 = new BasicDBObject();
        index5.put(MongoConstants.CONF_OWNER, 1);
        DBObject index6 = new BasicDBObject();
        index6.put(MongoConstants.IM_ID, 1);
        DBObject index7 = new BasicDBObject();
        index7.put(MongoConstants.ALT_IM_ID, 1);
        DBObject index8 = new BasicDBObject();
        index8.put(MongoConstants.IM_GROUP, 1);
        DBObject index9 = new BasicDBObject();
        index9.put(MongoConstants.ENTITY_NAME, 1);

        entity.ensureIndex(index1);
        entity.ensureIndex(index2);
        entity.ensureIndex(index3);
        entity.ensureIndex(index4);
        entity.ensureIndex(index5);
        entity.ensureIndex(index6);
        entity.ensureIndex(index7);
        entity.ensureIndex(index8);
        entity.ensureIndex(index9);

        return entity;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setExternalAliases(ExternalAliases externalAliases) {
        m_externalAliases = externalAliases;
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

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    @Override
    public boolean setup(SetupManager manager) {
        String id = "replication-" + new VersionInfo().getVersion();
        if (manager.isFalse(id)) {
            replicateAllData();
            manager.setTrue(id);
        }
        return true;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
}
