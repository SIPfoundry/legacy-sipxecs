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

import java.io.File;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.util.Collection;
import java.util.Map;
import java.util.Set;

import com.mongodb.BasicDBObject;
import com.mongodb.CommandResult;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.Mongo;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.commons.userdb.ValidUsers;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.logging.AuditLogContext;

import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.common.ReplicableProvider;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.xmlrpc.ApiProvider;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcRemoteException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

import static org.sipfoundry.commons.mongo.MongoConstants.ID;

public class ReplicationManagerImpl extends HibernateDaoSupport implements ReplicationManager, BeanFactoryAware {
    private static final int PERMISSIONS = 0644;
    private static final Log LOG = LogFactory.getLog(ReplicationManagerImpl.class);
    private static final String HOST = "localhost";
    private static final int PORT = 27017;
    private static final String DB_NAME = "imdb";
    private static final String ENTITY_COLLECTION_NAME = "entity";
    private static final String NODE_COLLECTION_NAME = "node";
    private static final String REPLICATION_FAILED = "Replication: insert/update failed - ";
    private static final String REPLICATION_FAILED_REMOVE = "Replication: delete failed - ";
    private static final String UNABLE_OPEN_MONGO = "Unable to open mongo connection on: ";
    private static final String LOCATION_REGISTRATION = "Location registration in db";
    private static final String DATABASE_REGENERATION = "Database Regeneration";
    private static final String COLON = ":";
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

    private Mongo m_mongoInstance;
    private DBCollection m_datasetCollection;

    private boolean m_enabled = true;

    private ApiProvider<FileApi> m_fileApiProvider;
    private LocationsManager m_locationsManager;
    private AuditLogContext m_auditLogContext;
    private ListableBeanFactory m_beanFactory;
    private CoreContext m_coreContext;
    private ForwardingContext m_forwardingContext;
    private ExternalAliases m_externalAliases;
    private DataSetGenerator m_dataSetGenerator;

    private void initMongo() throws Exception {
        if (m_mongoInstance == null) {
            try {
                m_mongoInstance = new Mongo(HOST, PORT);
                // defaults - the entity DB;
                DB datasetDb = m_mongoInstance.getDB(DB_NAME);
                m_datasetCollection = datasetDb.getCollection(ENTITY_COLLECTION_NAME);
            } catch (Exception e) {
                LOG.error(UNABLE_OPEN_MONGO + HOST + COLON + PORT);
                throw (e);
            }
        }
    }

    private Mongo initMongo(Location location) throws Exception {
        try {
            Mongo mongoInstance = new Mongo(location.getAddress(), PORT);
            return mongoInstance;
        } catch (Exception e) {
            LOG.error(UNABLE_OPEN_MONGO + HOST + COLON + PORT);
            throw (e);
        }
    }

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

    public void dropDb() throws Exception {
        initMongo();
        m_datasetCollection.drop();
    }

    /*
     * Get all replicable entities and replicate them; this is far better than getting the
     * DataSet.values and generating for each of them. Treat users differently since we have the
     * DaoUtils.forAllUsersDo method that should be used.
     */
    @Override
    public void replicateAllData() {
        Location primary = m_locationsManager.getPrimaryLocation();
        try {
            Long start = System.currentTimeMillis();
            dropDb(); // this calls initMongo()
            Closure<User> closure = new Closure<User>() {
                @Override
                public void execute(User user) {
                    replicateEntity(user);
                    CallSequence cs = m_forwardingContext.getCallSequenceForUser(user);
                    if (!cs.getRings().isEmpty()) {
                        replicateEntity(cs);
                    }
                    getHibernateTemplate().clear(); //clear the H session (see XX-9741)
                }

            };
            DaoUtils.forAllUsersDo(m_coreContext, closure);
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
            Long end = System.currentTimeMillis();
            LOG.info("Regeneration of database completed in " + (end - start) / 1000 + SECONDS + (end - start)
                    / 1000 / 60 + MINUTES);

            m_auditLogContext.logReplicationMongo(DATABASE_REGENERATION, primary);
        } catch (Exception e) {
            m_auditLogContext.logReplicationMongoFailed(DATABASE_REGENERATION, primary, e);
            LOG.error("Regeneration of database failed", e);
            throw new UserException(e);
        }
    }

    @Override
    public void replicateEntity(Replicable entity) {
        String name = (entity.getName() != null) ? entity.getName() : entity.toString();
        try {
            Long start = System.currentTimeMillis();
            initMongo();
            m_dataSetGenerator.setDbCollection(m_datasetCollection);
            DBObject top = m_dataSetGenerator.findOrCreate(entity);
            Set<DataSet> dataSets = entity.getDataSets();
            for (DataSet dataSet : dataSets) {
                replicateEntity(entity, dataSet, top);
            }
            Long end = System.currentTimeMillis();
            LOG.info(REPLICATION_INS_UPD + name + IN + (end - start) + MS);
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED + name, e);
            throw new UserException(REPLICATION_FAILED + entity.getName(), e);
        }
    }

    @Override
    public void replicateEntity(Replicable entity, DataSet dataSet) {
        String name = (entity.getName() != null) ? entity.getName() : entity.toString();
        try {
            Long start = System.currentTimeMillis();
            initMongo();
            m_dataSetGenerator.setDbCollection(m_datasetCollection);
            DBObject top = m_dataSetGenerator.findOrCreate(entity);
            replicateEntity(entity, dataSet, top);
            Long end = System.currentTimeMillis();
            LOG.info(REPLICATION_INS_UPD + name + IN + (end - start) + MS);
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED + name, e);
            throw new UserException(REPLICATION_FAILED + entity.getName(), e);
        }
    }

    private void replicateEntity(Replicable entity, DataSet dataSet, DBObject top) {
        String beanName = dataSet.getBeanName();
        final DataSetGenerator generator = m_beanFactory.getBean(beanName, DataSetGenerator.class);
        generator.setDbCollection(m_datasetCollection);
        generator.generate(entity, top);
        LOG.debug("Entity " + entity.getName() + "updated.");
    }

    @Override
    public void replicateAllData(final DataSet ds) {
        try {
            Long start = System.currentTimeMillis();
            dropDb(); // this calls initMongo()
            Map<String, ReplicableProvider> beanMap = m_beanFactory.getBeansOfType(ReplicableProvider.class);
            for (ReplicableProvider provider : beanMap.values()) {
                for (Replicable entity : provider.getReplicables()) {
                    if (!entity.getDataSets().contains(ds)) {
                        continue;
                    }
                    m_dataSetGenerator.setDbCollection(m_datasetCollection);
                    DBObject top = m_dataSetGenerator.findOrCreate(entity);
                    replicateEntity(entity, ds, top);
                }
            }
            Closure<User> closure = new Closure<User>() {
                @Override
                public void execute(User user) {
                    m_dataSetGenerator.setDbCollection(m_datasetCollection);
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
    public void replicateLocation(Location location) {
        try {
            if (location.isRegistered()) {
                initMongo();
                DB datasetDb = m_mongoInstance.getDB(DB_NAME);
                DBCollection nodeCollection = datasetDb.getCollection(NODE_COLLECTION_NAME);
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
                m_auditLogContext.logReplicationMongo(LOCATION_REGISTRATION, location);
            }
        } catch (Exception e) {
            m_auditLogContext.logReplicationMongoFailed(LOCATION_REGISTRATION, location, e);
            throw new UserException("Cannot register location in mongo db: " + e);
        }
    }

    @Override
    public void removeEntity(Replicable entity) {
        try {
            initMongo();
            DB datasetDb = m_mongoInstance.getDB(DB_NAME);
            DBCollection datasetCollection = datasetDb.getCollection(ENTITY_COLLECTION_NAME);
            String id = DataSetGenerator.getEntityId(entity);
            DBObject search = new BasicDBObject();
            search.put(ID, id);
            DBCursor cursor = datasetCollection.find(search);
            DBObject top = new BasicDBObject();
            if (!cursor.hasNext()) {
                top.put(ID, id);
            } else {
                top = cursor.next();
            }
            datasetCollection.remove(top).getError();
            LOG.info("Replication: removed " + entity.getName());
        } catch (Exception e) {
            LOG.error(REPLICATION_FAILED_REMOVE + entity.getName(), e);
            throw new UserException(REPLICATION_FAILED_REMOVE + entity.getName(), e);
        }
    }

    @Override
    public void removeLocation(Location location) {
        try {
            if (location.isRegistered()) {
                initMongo();
                DB datasetDb = m_mongoInstance.getDB(DB_NAME);
                DBCollection nodeCollection = datasetDb.getCollection(NODE_COLLECTION_NAME);
                DBObject search = new BasicDBObject();
                search.put(ID, location.getId());
                DBCursor cursor = nodeCollection.find(search);
                DBObject node = new BasicDBObject();
                if (cursor.hasNext()) {
                    node = cursor.next();
                }
                nodeCollection.remove(node);
            }
        } catch (Exception e) {
            throw new UserException("Cannot unregister location in mongo db: " + e);
        }
    }

    @Override
    public void resyncSlave(Location location) {
        try {
            Mongo mongo = initMongo(location);
            DB adminDb = mongo.getDB("admin");
            CommandResult cr = adminDb.command("resync");
            double ok = (Double) cr.get("ok");
            if (ok == 0) {
                LOG.error("Replication: resync - " + cr.get("errmsg"));
            } else {
                LOG.info("Replication: resync started - " + location.getAddress());
            }
        } catch (Exception e) {
            LOG.error("Replication: resync failed - " + location.getAddress(), e);
        }
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

    public void addPermission(Permission permission) {
        try {
            initMongo();
            DBCursor users = ValidUsers.INSTANCE.getEntitiesWithPermissions();
            for (DBObject user : users) {
                Collection<String> prms = (Collection<String>) user.get(MongoConstants.PERMISSIONS);
                prms.add(permission.getName());
                user.put(MongoConstants.PERMISSIONS, prms);
                m_datasetCollection.save(user);
            }
        } catch (Exception e) {
            LOG.error(ERROR_PERMISSION, e);
            throw new UserException(ERROR_PERMISSION, e);
        }
    }

    public void removePermission(Permission permission) {
        try {
            initMongo();
            DBCursor users = ValidUsers.INSTANCE.getEntitiesWithPermission(permission.getName());
            for (DBObject user : users) {
                Collection<String> prms = (Collection<String>) user.get(MongoConstants.PERMISSIONS);
                prms.remove(permission.getName());
                user.put(MongoConstants.PERMISSIONS, prms);
                m_datasetCollection.save(user);
            }
        } catch (Exception e) {
            LOG.error(ERROR_PERMISSION, e);
            throw new UserException(ERROR_PERMISSION, e);
        }
    }

    @Override
    public boolean replicateFile(Location[] locations, ConfigurationFile file) {

        if (!m_enabled) {
            return true;
        }
        boolean success = false;
        for (int i = 0; i < locations.length; i++) {
            if (!locations[i].isPrimary()) {
                continue;
            }
            LOG.info("Writing " + file.getName() + " to primary location: " + locations[i].getFqdn());
            File f = new File(file.getPath());
            try {
                f.createNewFile();
                FileWriter writer = new FileWriter(f);
                file.write(writer, locations[i]);
                writer.close();
                success = true;
                m_auditLogContext.logReplication(file.getName(), locations[i]);
            } catch (IOException e) {
                LOG.error("Error writing: " + f.getAbsolutePath());
                m_auditLogContext.logReplicationFailed(file.getName(), locations[i], e);
                throw new RuntimeException(e);
            }
        }
        for (int i = 0; i < locations.length; i++) {
            if (!locations[i].isRegistered() || locations[i].isPrimary()) {
                continue;
            }
            if (!file.isReplicable(locations[i])) {
                LOG.info("File " + file.getName() + " cannot be replicated on location: " + locations[i].getFqdn());
                success = true;
                continue;
            }
            try {
                LOG.info("Transferring file " + file.getName() + " to secondary location: " + locations[i].getFqdn());
                StringWriter stringWriter = new StringWriter();
                IOUtils.copy(new FileInputStream(new File(file.getPath())), stringWriter);

                String content = encodeBase64(stringWriter.toString().getBytes());

                FileApi api = m_fileApiProvider.getApi(locations[i].getProcessMonitorUrl());
                success = api.replace(getHostname(), file.getPath(), PERMISSIONS, content);
                if (success) {
                    m_auditLogContext.logReplication(file.getName(), locations[i]);
                } else {
                    m_auditLogContext.logReplicationFailed(file.getName(), locations[i], null);
                }
            } catch (XmlRpcRemoteException e) {
                LOG.error("File replication failed: " + file.getName(), e);
                m_auditLogContext.logReplicationFailed(file.getName(), locations[i], e);
            } catch (UnsupportedEncodingException e) {
                LOG.error("UTF-8 encoding should be always supported.");
                m_auditLogContext.logReplicationFailed(file.getName(), locations[i], e);
                throw new RuntimeException(e);
            } catch (IOException e) {
                LOG.error(e);
                m_auditLogContext.logReplicationFailed(file.getName(), locations[i], e);
                throw new RuntimeException(e);
            }
        }
        return success;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    private String getHostname() {
        return m_locationsManager.getPrimaryLocation().getFqdn();
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

}
