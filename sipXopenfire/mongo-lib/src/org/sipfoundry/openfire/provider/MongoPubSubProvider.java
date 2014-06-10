/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 */
package org.sipfoundry.openfire.provider;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Date;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.StringTokenizer;

import org.apache.commons.lang.StringUtils;
import org.apache.log4j.Logger;
import org.bson.types.ObjectId;
import org.jivesoftware.openfire.cluster.ClusterManager;
import org.jivesoftware.openfire.pubsub.BasePubSubProvider;
import org.jivesoftware.openfire.pubsub.CollectionNode;
import org.jivesoftware.openfire.pubsub.DefaultNodeConfiguration;
import org.jivesoftware.openfire.pubsub.LeafNode;
import org.jivesoftware.openfire.pubsub.Node;
import org.jivesoftware.openfire.pubsub.NodeAffiliate;
import org.jivesoftware.openfire.pubsub.NodeSubscription;
import org.jivesoftware.openfire.pubsub.PubSubService;
import org.jivesoftware.openfire.pubsub.PublishedItem;
import org.jivesoftware.openfire.pubsub.cluster.FlushTask;
import org.jivesoftware.openfire.pubsub.models.AccessModel;
import org.jivesoftware.openfire.pubsub.models.PublisherModel;
import org.jivesoftware.util.JiveGlobals;
import org.jivesoftware.util.LinkedListNode;
import org.jivesoftware.util.cache.CacheFactory;
import org.sipfoundry.commons.util.UnfortunateLackOfSpringSupportFactory;
import org.xmpp.packet.JID;

import com.mongodb.BasicDBObject;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBObject;
import com.mongodb.QueryOperators;
import com.mongodb.WriteResult;

public class MongoPubSubProvider extends BasePubSubProvider {
    private static final Logger log = Logger.getLogger(MongoPubSubProvider.class);

    private static final String COLLECTION_NAME = "ofPubsubNode";
    private static final String AFFILIATION_COLLECTION_NAME = "ofPubsubAffiliation";
    private static final String DEFAULT_CFG_COLLECTION_NAME = "ofPubsubDefaultConf";
    private static final String ITEM_COLLECTION_NAME = "ofPubsubItem";
    private static final String NODE_GROUP_COLLECTION_NAME = "ofPubsubNodeGroups";
    private static final String NODE_JID_COLLECTION_NAME = "ofPubsubNodeJIDs";
    private static final String SUBSCRIPTIONS_COLLECTION_NAME = "ofPubsubSubscription";

    @Override
    public void createNode(Node node) {
        DBObject toInsert = nodeToDBObject(node);

        getDefaultCollection().insert(toInsert);

        saveAssociatedElements(node);
    }

    @Override
    public void updateNode(Node node) {
        DBObject query = new BasicDBObject();
        query.put("serviceId", node.getService().getServiceID());
        query.put("nodeId", node.getNodeID());

        DBObject update = nodeToDBObject(node);

        getDefaultCollection().update(query, update);

        DBObject toDelete = new BasicDBObject();
        toDelete.put("serviceId", node.getService().getServiceID());
        toDelete.put("nodeId", node.getNodeID());

        // delete old associated elements
        getCollection(NODE_JID_COLLECTION_NAME).remove(toDelete);
        getCollection(NODE_GROUP_COLLECTION_NAME).remove(toDelete);

        saveAssociatedElements(node);
    }

    @Override
    public boolean removeNode(Node node) {
        DBObject toDelete = new BasicDBObject();
        toDelete.put("serviceId", node.getService().getServiceID());
        toDelete.put("nodeId", node.getNodeID());

        getDefaultCollection().remove(toDelete);
        getCollection(NODE_JID_COLLECTION_NAME).remove(toDelete);
        getCollection(NODE_GROUP_COLLECTION_NAME).remove(toDelete);
        getCollection(ITEM_COLLECTION_NAME).remove(toDelete);
        getCollection(AFFILIATION_COLLECTION_NAME).remove(toDelete);
        getCollection(SUBSCRIPTIONS_COLLECTION_NAME).remove(toDelete);

        return true;
    }

    @Override
    public void loadNodes(PubSubService service) {
        Map<String, Node> nodes = new HashMap<String, Node>();

        DBObject query = new BasicDBObject();

        query.put("serviceId", service.getServiceID());

        for (DBObject nodeObj : getDefaultCollection().find(query)) {
            loadNode(service, nodes, nodeObj);
        }

        loadNodeDependencies(service, nodes, query);
    }

    @Override
    public void loadNode(PubSubService service, String nodeId) {
        Map<String, Node> nodes = new HashMap<String, Node>();

        // Get all non-leaf nodes (to ensure parent nodes are loaded before
        // their children)
        DBObject query = new BasicDBObject();

        query.put("serviceID", service.getServiceID());
        query.put("nodeID", nodeId);

        Map<String, String> parentMapping = new HashMap<String, String>();

        // Rebuild loaded non-leaf nodes
        for (DBObject node : getDefaultCollection().find(query)) {
            loadNode(service, nodes, parentMapping, node);
        }
        String parentId = parentMapping.get(nodeId);

        if (parentId != null) {
            CollectionNode parent = (CollectionNode) service.getNode(parentId);

            if (parent == null) {
                log.error("Could not find parent node " + parentId + " for node " + nodeId);
            } else {
                nodes.get(nodeId).changeParent(parent);
            }
        }

        loadNodeDependencies(service, nodes, query);
    }

    @Override
    public void saveAffiliation(Node node, NodeAffiliate affiliate, boolean create) {
        if (create) {
            DBObject toInsert = new BasicDBObject();

            toInsert.put("serviceId", node.getService().getServiceID());
            toInsert.put("nodeId", node.getNodeID());
            toInsert.put("jid", affiliate.getJID().toString());
            toInsert.put("affiliation", affiliate.getAffiliation().name());

            getCollection(AFFILIATION_COLLECTION_NAME).insert(toInsert);
        } else {
            DBObject query = new BasicDBObject();

            query.put("serviceId", node.getService().getServiceID());
            query.put("nodeId", node.getNodeID());
            query.put("jid", affiliate.getJID().toString());

            DBObject update = new BasicDBObject();

            update.put("affiliation", affiliate.getAffiliation().name());

            getCollection(AFFILIATION_COLLECTION_NAME).update(query, new BasicDBObject("$set", update));
        }
    }

    @Override
    public void removeAffiliation(Node node, NodeAffiliate affiliate) {
        DBObject toDelete = new BasicDBObject();

        toDelete.put("serviceId", node.getService().getServiceID());
        toDelete.put("nodeId", node.getNodeID());
        toDelete.put("jid", affiliate.getJID().toString());

        getCollection(AFFILIATION_COLLECTION_NAME).remove(toDelete);
    }

    @Override
    public void saveSubscription(Node node, NodeSubscription subscription, boolean create) {
        if (create) {
            DBObject toInsert = subscriptionToDBObject(node, subscription);

            getCollection(SUBSCRIPTIONS_COLLECTION_NAME).insert(toInsert);

            subscription.setSavedToDB(true);
        } else {
            if (NodeSubscription.State.none == subscription.getState()) {
                removeSubscription(subscription);
            } else {
                DBObject query = new BasicDBObject();

                query.put("serviceId", node.getService().getServiceID());
                query.put("nodeId", node.getNodeID());
                query.put("id", subscription.getID());

                DBObject update = subscriptionToDBObject(node, subscription);

                getCollection(SUBSCRIPTIONS_COLLECTION_NAME).update(query, new BasicDBObject("$set", update));
            }
        }
    }

    @Override
    public void removeSubscription(NodeSubscription subscription) {
        Node node = subscription.getNode();
        DBObject toDelete = new BasicDBObject();

        toDelete.put("serviceId", node.getService().getServiceID());
        toDelete.put("nodeId", node.getNodeID());
        toDelete.put("id", subscription.getID());

        getCollection(SUBSCRIPTIONS_COLLECTION_NAME).remove(toDelete);
    }

    @Override
    public DefaultNodeConfiguration loadDefaultConfiguration(PubSubService service, boolean isLeafType) {
        DefaultNodeConfiguration config = null;
        DBObject query = new BasicDBObject();

        query.put("serviceId", service.getServiceID());
        query.put("leaf", isLeafType);

        DBObject confObj = getCollection(DEFAULT_CFG_COLLECTION_NAME).findOne(query);

        if (confObj != null) {
            config = dbObjectToConfig(confObj, isLeafType);
        }

        return config;
    }

    @Override
    public String loadPEPServiceFromDB(String jid) {
        String id = null;
        DBObject query = new BasicDBObject();

        query.put("serviceId", jid);

        DBObject fields = new BasicDBObject();

        fields.put("serviceId", 1);

        if (getDefaultCollection().findOne(query, fields) != null) {
            // if there's a node, we already know the service id
            id = jid;
        }

        return id;
    }

    @Override
    public void createDefaultConfiguration(PubSubService service, DefaultNodeConfiguration config) {
        DBObject toInsert = configToDBObject(service, config);

        getCollection(DEFAULT_CFG_COLLECTION_NAME).insert(toInsert);
    }

    @Override
    public void updateDefaultConfiguration(PubSubService service, DefaultNodeConfiguration config) {
        DBObject query = new BasicDBObject();
        query.put("serviceId", service.getServiceID());
        query.put("leaf", config.isLeaf());

        DBObject update = configToDBObject(service, config);

        getCollection(DEFAULT_CFG_COLLECTION_NAME).update(query, new BasicDBObject("$set", update));
    }

    /**
     * Flush the cache of items to be persisted and deleted.
     */
    @Override
    public void flushPendingItems() {
        flushPendingItems(ClusterManager.isClusteringEnabled());
    }

    /**
     * Flush the cache of items to be persisted and deleted.
     *
     * @param sendToCluster If true, delegate to cluster members, otherwise local only
     */
    @Override
    public void flushPendingItems(boolean sendToCluster) {
        if (sendToCluster) {
            CacheFactory.doSynchronousClusterTask(new FlushTask(), false);
        }

        LinkedListNode<PublishedItem> addItem = getFirstToAdd();
        LinkedListNode<PublishedItem> delItem = getFirstToDelete();
        LinkedListNode<PublishedItem> addLast = getLastToAdd();
        LinkedListNode<PublishedItem> delLast = getLastToDelete();

        if (addItem == null && delItem == null) {
            return; // nothing to do for this cluster member
        }

        movePendingToAdd();

        DBCollection itemCollection = getCollection(ITEM_COLLECTION_NAME);
        if (delItem != null) {
            LinkedListNode<PublishedItem> delHead = delLast.next;

            // delete first (to remove possible duplicates), then add new items
            while (delItem != delHead) {
                DBObject toDelete = new BasicDBObject();
                PublishedItem item = delItem.object;

                toDelete.put("serviceID", item.getNode().getService().getServiceID());
                toDelete.put("nodeID", item.getNode().getNodeID());
                toDelete.put("id", item.getID());

                itemCollection.remove(toDelete);
            }
        }

        if (addItem != null) {
            LinkedListNode<PublishedItem> addHead = addLast.next;
            Set<DBObject> toInsertSet = new HashSet<DBObject>();

            while (addItem != addHead) {
                DBObject toInsert = new BasicDBObject();
                PublishedItem item = addItem.object;

                toInsert.put("serviceID", item.getNode().getService().getServiceID());
                toInsert.put("nodeID", item.getNodeID());
                toInsert.put("id", item.getID());
                toInsert.put("jid", item.getPublisher().toString());
                toInsert.put("creationDate", item.getCreationDate().getTime());
                toInsert.put("payload", item.getPayloadXML());
                itemCollection.insert(toInsert);

                toInsertSet.add(toInsert);

                addItem = addItem.next;
            }

            itemCollection.insert(toInsertSet.toArray(new BasicDBObject[toInsertSet.size()]));
        }
    }

    @Override
    public void loadSubscription(Node node, String subId) {
        Map<String, Node> nodes = new HashMap<String, Node>();
        nodes.put(node.getNodeID(), node);

        DBObject query = new BasicDBObject();

        query.put("serviceID", node.getService().getServiceID());
        query.put("nodeID", node.getNodeID());
        query.put("id", subId);

        DBObject subscriptionObj = getDefaultCollection().findOne(query);
        if (subscriptionObj != null) {
            loadSubscriptions(nodes, subscriptionObj);
        }
    }

    @Override
    public List<PublishedItem> getPublishedItems(LeafNode node, int maxRows) {
        safeFlushPendingItems();

        int maxPublished = node.getMaxPublishedItems();
        int max = Math.min(getMaxRowsFetch(), maxPublished);

        java.util.LinkedList<PublishedItem> results = new java.util.LinkedList<PublishedItem>();
        boolean descending = JiveGlobals.getBooleanProperty("xmpp.pubsub.order.descending", false);

        // Get published items of the specified node
        DBObject query = new BasicDBObject();

        query.put("serviceID", node.getService().getServiceID());
        query.put("nodeID", node.getNodeID());

        DBObject nodeSort = new BasicDBObject("creationDate", -1);

        int counter = 0;

        // Rebuild loaded published items
        for (DBObject itemObj : getCollection(ITEM_COLLECTION_NAME).find(query).sort(nodeSort)) {
            String itemID = (String) itemObj.get("id");
            JID publisher = new JID((String) itemObj.get("jid"));
            Date creationDate = new Date((Long) itemObj.get("creationDate"));
            // Create the item
            PublishedItem item = new PublishedItem(node, publisher, itemID, creationDate);
            // Add the extra fields to the published item
            String payload = (String) itemObj.get("payload");
            if (payload != null) {
                item.setPayloadXML(payload);
            }
            // Add the published item to the node
            if (descending) {
                results.add(item);
            } else {
                results.addFirst(item);
            }
            if (++counter > max) {
                break;
            }
        }

        return results;
    }

    @Override
    public PublishedItem getLastPublishedItem(LeafNode node) {
        safeFlushPendingItems();

        PublishedItem item = null;

        DBObject query = new BasicDBObject();

        query.put("serviceID", node.getService().getServiceID());
        query.put("nodeID", node.getNodeID());

        DBObject nodeSort = new BasicDBObject("creationDate", -1);
        DBObject itemObj = getCollection(ITEM_COLLECTION_NAME).findOne(query, null, nodeSort);

        if (itemObj != null) {
            String itemID = (String) itemObj.get("id");
            JID publisher = new JID((String) itemObj.get("jid"));
            Date creationDate = new Date((Long) itemObj.get("creationDate"));
            // Create the item
            item = new PublishedItem(node, publisher, itemID, creationDate);
            // Add the extra fields to the published item
            String payload = (String) itemObj.get("payload");
            if (payload != null) {
                item.setPayloadXML(payload);
            }
        }

        return item;
    }

    @Override
    public PublishedItem loadItem(LeafNode node, String itemID) {
        PublishedItem result = null;

        flushPendingItems();

        DBObject query = new BasicDBObject();

        query.put("serviceID", node.getService().getServiceID());
        query.put("nodeID", node.getNodeID());
        query.put("id", itemID);

        DBObject itemObj = getCollection(ITEM_COLLECTION_NAME).findOne(query);

        if (itemObj != null) {
            JID publisher = new JID((String) itemObj.get("jid"));
            Date creationDate = new Date((Long) itemObj.get("creationDate"));
            // Create the item
            result = new PublishedItem(node, publisher, itemID, creationDate);
            // Add the extra fields to the published item
            String payload = (String) itemObj.get("payload");
            if (payload != null) {
                result.setPayloadXML(payload);
            }
            log.debug("Loaded item into cache from DB");
        }

        return result;
    }

    @Override
    protected boolean purgeNodeFromDB(LeafNode leafNode) {
        flushPendingItems(ClusterManager.isClusteringEnabled());

        DBObject toDelete = new BasicDBObject();

        toDelete.put("serviceID", leafNode.getService().getServiceID());
        toDelete.put("nodeID", leafNode.getNodeID());

        WriteResult result = getCollection(ITEM_COLLECTION_NAME).remove(toDelete);
        if (result.getError() != null) {
            evictFromCache(leafNode);
        }

        return result.getError() != null;
    }

    /**
     * Purges all items from the database that exceed the defined item count on all nodes.
     */
    @Override
    protected void purgeItems() {
        DBObject nodeQuery = new BasicDBObject();

        nodeQuery.put("leaf", true);
        nodeQuery.put("persistItems", true);
        nodeQuery.put("maxItems", new BasicDBObject(QueryOperators.GT, 0));

        for (DBObject nodeObj : getDefaultCollection().find(nodeQuery)) {
            String svcId = (String) nodeObj.get("serviceID");
            String nodeId = (String) nodeObj.get("nodeID");
            int maxItems = (Integer) nodeObj.get("maxItems");

            DBObject itemQuery = new BasicDBObject();

            itemQuery.put("serviceID", svcId);
            itemQuery.put("nodeID", nodeId);

            DBObject itemSort = new BasicDBObject();

            itemSort.put("creationDate", 1);

            // skip oldest maxItems items, delete the rest
            for (DBObject itemObject : getCollection(ITEM_COLLECTION_NAME).find(itemQuery).sort(itemSort)
                    .skip(maxItems)) {
                getCollection(ITEM_COLLECTION_NAME).remove(itemObject);
            }
        }
    }

    private static void saveAssociatedElements(Node node) {
        DBCollection nodeJidCollection = getCollection(NODE_JID_COLLECTION_NAME);

        for (JID jid : node.getContacts()) {
            DBObject toInsert = nodeJidToDBObject(node, jid, "contacts");

            nodeJidCollection.insert(toInsert);
        }
        for (JID jid : node.getReplyRooms()) {
            DBObject toInsert = nodeJidToDBObject(node, jid, "replyRooms");

            nodeJidCollection.insert(toInsert);
        }
        for (JID jid : node.getReplyTo()) {
            DBObject toInsertContact = nodeJidToDBObject(node, jid, "replyTo");

            nodeJidCollection.insert(toInsertContact);
        }
        if (node.isCollectionNode()) {
            for (JID jid : ((CollectionNode) node).getAssociationTrusted()) {
                DBObject toInsert = nodeJidToDBObject(node, jid, "associationTrusted");

                nodeJidCollection.insert(toInsert);
            }
        }

        for (String groupName : node.getRosterGroupsAllowed()) {
            DBObject toInsert = new BasicDBObject();

            toInsert.put("serviceId", node.getService().getServiceID());
            toInsert.put("nodeId", node.getNodeID());
            toInsert.put("rosterGroup", groupName);

            getCollection(NODE_GROUP_COLLECTION_NAME).insert(toInsert);
        }
    }

    private static DBObject nodeToDBObject(Node node) {
        DBObject nodeObj = new BasicDBObject();

        nodeObj.put("serviceId", node.getService().getServiceID());
        if (!StringUtils.isEmpty(node.getNodeID())) {
            nodeObj.put("nodeId", node.getNodeID());
        } else {
            nodeObj.put("nodeId", new ObjectId().toString());
        }
        if (node.isCollectionNode()) {
            nodeObj.put("leaf", false);
            nodeObj.put("maxPayloadSize", 0);
            nodeObj.put("persistItems", false);
            nodeObj.put("maxItems", 0);
        } else {
            nodeObj.put("leaf", true);
            nodeObj.put("maxPayloadSize", ((LeafNode) node).getMaxPayloadSize());
            nodeObj.put("persistItems", ((LeafNode) node).isPersistPublishedItems());
            nodeObj.put("maxItems", ((LeafNode) node).getMaxPublishedItems());
        }
        nodeObj.put("creationDate", node.getCreationDate());
        nodeObj.put("modificationDate", node.getModificationDate());
        nodeObj.put("parent", node.getParent() != null ? node.getParent().getNodeID() : null);
        nodeObj.put("deliverPayloads", node.isPayloadDelivered());
        nodeObj.put("notifyConfigChanges", node.isNotifiedOfConfigChanges());
        nodeObj.put("notifyDelete", node.isNotifiedOfDelete());
        nodeObj.put("notifyRetract", node.isNotifiedOfRetract());
        nodeObj.put("presenceBased", node.isPresenceBasedDelivery());
        nodeObj.put("sendItemSubscribe", node.isSendItemSubscribe());
        nodeObj.put("publisherModel", node.getPublisherModel().getName());
        nodeObj.put("subscriptionEnabled", node.isSubscriptionEnabled());
        nodeObj.put("configSubscription", node.isSubscriptionConfigurationRequired());
        nodeObj.put("accessModel", node.getAccessModel().getName());
        nodeObj.put("payloadType", node.getPayloadType());
        nodeObj.put("bodyXslt", node.getBodyXSLT());
        nodeObj.put("dataformXslt", node.getDataformXSLT());
        nodeObj.put("creator", node.getCreator().toString());
        nodeObj.put("description", node.getDescription());
        nodeObj.put("language", node.getLanguage());
        nodeObj.put("name", node.getName());
        nodeObj.put("replyPolicy", node.getReplyPolicy() != null ? node.getReplyPolicy().name() : null);
        if (node.isCollectionNode()) {
            nodeObj.put("associationPolicy", ((CollectionNode) node).getAssociationPolicy().name());
            nodeObj.put("maxLeafNodes", ((CollectionNode) node).getMaxLeafNodes());
        } else {
            nodeObj.put("associationPolicy", null);
            nodeObj.put("maxLeafNodes", 0);
        }

        return nodeObj;
    }

    private static DBObject configToDBObject(PubSubService service, DefaultNodeConfiguration config) {
        DBObject dbObj = new BasicDBObject();

        dbObj.put("serviceId", service.getServiceID());
        dbObj.put("leaf", config.isLeaf());
        dbObj.put("deliverPayloads", config.isDeliverPayloads());
        dbObj.put("maxPayloadSize", config.getMaxPayloadSize());
        dbObj.put("persistItems", config.isPersistPublishedItems());
        dbObj.put("maxItems", config.getMaxPublishedItems());
        dbObj.put("notifyConfigChanges", config.isNotifyConfigChanges());
        dbObj.put("notifyDelete", config.isNotifyDelete());
        dbObj.put("notifyRetract", config.isNotifyRetract());
        dbObj.put("presenceBased", config.isPresenceBasedDelivery());
        dbObj.put("sendItemSubscribe", config.isSendItemSubscribe());
        dbObj.put("publisherModel", config.getPublisherModel().getName());
        dbObj.put("subscriptionEnabled", config.isSubscriptionEnabled());
        dbObj.put("accessModel", config.getAccessModel().getName());
        dbObj.put("language", config.getLanguage());
        dbObj.put("replyPolicy", config.getReplyPolicy() != null ? config.getReplyPolicy().name() : null);
        dbObj.put("associationPolicy", config.getAssociationPolicy().name());
        dbObj.put("maxLeafNodes", config.getMaxLeafNodes());

        return dbObj;
    }

    private static DefaultNodeConfiguration dbObjectToConfig(DBObject confObj, boolean isLeafType) {
        DefaultNodeConfiguration config = new DefaultNodeConfiguration(isLeafType);

        Boolean deliverPayloads = (Boolean) confObj.get("deliverPayloads");
        Integer maxPayloadSize = (Integer) confObj.get("maxPayloadSize");
        Boolean persistItems = (Boolean) confObj.get("persistItems");
        Integer maxItems = (Integer) confObj.get("maxItems");
        Boolean notifyConfigChanges = (Boolean) confObj.get("notifyConfigChanges");
        Boolean notifyDelete = (Boolean) confObj.get("notifyDelete");
        Boolean notifyRetract = (Boolean) confObj.get("notifyRetract");
        Boolean presenceBased = (Boolean) confObj.get("presenceBased");
        Boolean sendItemSubscribe = (Boolean) confObj.get("sendItemSubscribe");
        String publisherModel = (String) confObj.get("publisherModel");
        Boolean subscriptionEnabled = (Boolean) confObj.get("subscriptionEnabled");
        String accessModel = (String) confObj.get("accessModel");
        String language = (String) confObj.get("language");
        String replyPolicy = (String) confObj.get("replyPolicy");
        String associationPolicy = (String) confObj.get("associationPolicy");
        Integer maxLeafNodes = (Integer) confObj.get("maxLeafNodes");

        config.setDeliverPayloads(deliverPayloads);
        config.setMaxPayloadSize(maxPayloadSize);
        config.setPersistPublishedItems(persistItems);
        config.setMaxPublishedItems(maxItems);
        config.setNotifyConfigChanges(notifyConfigChanges);
        config.setNotifyDelete(notifyDelete);
        config.setNotifyRetract(notifyRetract);
        config.setPresenceBasedDelivery(presenceBased);
        config.setSendItemSubscribe(sendItemSubscribe);
        config.setPublisherModel(PublisherModel.valueOf(publisherModel));
        config.setSubscriptionEnabled(subscriptionEnabled);
        config.setAccessModel(AccessModel.valueOf(accessModel));
        config.setLanguage(language);
        config.setReplyPolicy(replyPolicy != null ? Node.ItemReplyPolicy.valueOf(replyPolicy) : null);
        config.setAssociationPolicy(CollectionNode.LeafNodeAssociationPolicy.valueOf(associationPolicy));
        config.setMaxLeafNodes(maxLeafNodes);

        return config;
    }

    private static DBObject nodeJidToDBObject(Node node, JID jid, String type) {
        DBObject nodeObj = new BasicDBObject();

        nodeObj.put("serviceId", node.getService().getServiceID());
        nodeObj.put("nodeId", node.getNodeID());
        nodeObj.put("jid", jid.toString());
        nodeObj.put("associationType", type);

        return nodeObj;
    }

    private static DBObject subscriptionToDBObject(Node node, NodeSubscription subscription) {
        DBObject subscrObj = new BasicDBObject();

        subscrObj.put("serviceId", node.getService().getServiceID());
        subscrObj.put("nodeId", node.getNodeID());
        subscrObj.put("id", subscription.getID());
        subscrObj.put("jid", subscription.getJID().toString());
        subscrObj.put("owner", subscription.getOwner().toString());
        subscrObj.put("state", subscription.getState().name());
        subscrObj.put("deliver", subscription.shouldDeliverNotifications());
        subscrObj.put("digest", subscription.isUsingDigest());
        subscrObj.put("digestFrequency", subscription.getDigestFrequency());
        Date expireDate = subscription.getExpire();
        subscrObj.put("expire", expireDate != null ? expireDate.getTime() : null);
        subscrObj.put("includeBody", subscription.isIncludingBody());
        subscrObj.put("showValues", encodeWithComma(subscription.getPresenceStates()));
        subscrObj.put("subscriptionType", subscription.getType().name());
        subscrObj.put("subscriptionDepth", subscription.getDepth());
        subscrObj.put("keyword", subscription.getKeyword());
        return subscrObj;
    }

    private static void loadNode(PubSubService service, Map<String, Node> nodes, DBObject nodeObj) {
        loadNode(service, nodes, null, nodeObj);
    }

    private static void loadNode(PubSubService service, Map<String, Node> nodes, Map<String, String> parentMappings,
            DBObject nodeObj) {
        String nodeId = (String) nodeObj.get("nodeId");
        boolean leaf = (Boolean) nodeObj.get("leaf");
        String parent = (String) nodeObj.get("parent");

        if (parent != null && parentMappings != null) {
            parentMappings.put(nodeId, parent);
        }

        CollectionNode parentNode = null;
        if (parent != null && nodes.get(parent) == null) {
            return;
        }

        Date creationDate = (Date) nodeObj.get("creationDate");
        Date modificationDate = (Date) nodeObj.get("modificationDate");
        Boolean deliverPayloads = (Boolean) nodeObj.get("deliverPayloads");
        Integer maxPayloadSize = (Integer) nodeObj.get("maxPayloadSize");
        Boolean persistItems = (Boolean) nodeObj.get("persistItems");
        Integer maxItems = (Integer) nodeObj.get("maxItems");
        Boolean notifyConfigChanges = (Boolean) nodeObj.get("notifyConfigChanges");
        Boolean notifyDelete = (Boolean) nodeObj.get("notifyDelete");
        Boolean notifyRetract = (Boolean) nodeObj.get("notifyRetract");
        Boolean presenceBased = (Boolean) nodeObj.get("presenceBased");
        Boolean sendItemSubscribe = (Boolean) nodeObj.get("sendItemSubscribe");
        String publisherModel = (String) nodeObj.get("publisherModel");
        Boolean subscriptionEnabled = (Boolean) nodeObj.get("subscriptionEnabled");
        Boolean configSubscription = (Boolean) nodeObj.get("configSubscription");
        String accessModel = (String) nodeObj.get("accessModel");
        String payloadType = (String) nodeObj.get("payloadType");
        String bodyXslt = (String) nodeObj.get("bodyXslt");
        String dataformXslt = (String) nodeObj.get("dataformXslt");
        String creator = (String) nodeObj.get("creator");
        String description = (String) nodeObj.get("description");
        String language = (String) nodeObj.get("language");
        String name = (String) nodeObj.get("name");
        String replyPolicy = (String) nodeObj.get("replyPolicy");
        String associationPolicy = (String) nodeObj.get("associationPolicy");
        Integer maxLeafNodes = (Integer) nodeObj.get("maxLeafNodes");

        JID creatorJid = new JID(creator);
        Node node;
        if (leaf) {
            node = new LeafNode(service, parentNode, nodeId, creatorJid);
        } else {
            node = new CollectionNode(service, parentNode, nodeId, creatorJid);
        }

        node.setCreationDate(creationDate);
        node.setModificationDate(modificationDate);
        node.setPayloadDelivered(deliverPayloads);
        if (leaf) {
            ((LeafNode) node).setMaxPayloadSize(maxPayloadSize);
            ((LeafNode) node).setPersistPublishedItems(persistItems);
            ((LeafNode) node).setMaxPublishedItems(maxItems);
            ((LeafNode) node).setSendItemSubscribe(sendItemSubscribe);
        }
        node.setNotifiedOfConfigChanges(notifyConfigChanges);
        node.setNotifiedOfDelete(notifyDelete);
        node.setNotifiedOfRetract(notifyRetract);
        node.setPresenceBasedDelivery(presenceBased);
        node.setPublisherModel(PublisherModel.valueOf(publisherModel));
        node.setSubscriptionEnabled(subscriptionEnabled);
        node.setSubscriptionConfigurationRequired(configSubscription);
        node.setAccessModel(AccessModel.valueOf(accessModel));
        node.setPayloadType(payloadType);
        node.setBodyXSLT(bodyXslt);
        node.setDataformXSLT(dataformXslt);
        node.setDescription(description);
        node.setLanguage(language);
        node.setName(name);
        if (replyPolicy != null) {
            node.setReplyPolicy(Node.ItemReplyPolicy.valueOf(replyPolicy));
        }
        if (!leaf) {
            ((CollectionNode) node).setAssociationPolicy(CollectionNode.LeafNodeAssociationPolicy
                    .valueOf(associationPolicy));
            ((CollectionNode) node).setMaxLeafNodes(maxLeafNodes);
        }

        nodes.put(nodeId, node);
    }

    private static void loadNodeDependencies(PubSubService service, Map<String, Node> nodes, DBObject query) {
        for (DBObject nodeJidObj : getCollection(NODE_JID_COLLECTION_NAME).find(query)) {
            loadAssociatedJids(nodes, nodeJidObj);
        }

        for (DBObject nodeGroupObj : getCollection(NODE_GROUP_COLLECTION_NAME).find(query)) {
            loadAssociatedGroups(nodes, nodeGroupObj);
        }

        for (DBObject affiliationObj : getCollection(AFFILIATION_COLLECTION_NAME).find(query)) {
            loadAffiliations(nodes, affiliationObj);
        }

        for (DBObject subscriptionObj : getCollection(SUBSCRIPTIONS_COLLECTION_NAME).find(query)) {
            loadSubscriptions(nodes, subscriptionObj);
        }

        for (Node node : nodes.values()) {
            node.setSavedToDB(true);
            service.addNode(node);
        }
    }

    private static void loadAssociatedJids(Map<String, Node> nodes, DBObject nodeJidObj) {
        String nodeId = (String) nodeJidObj.get("nodeId");
        Node node = nodes.get(nodeId);
        if (node != null) {
            JID jid = new JID((String) nodeJidObj.get("jid"));
            String associationType = (String) nodeJidObj.get("associationType");
            if ("contacts".equals(associationType)) {
                node.addContact(jid);
            } else if ("replyRooms".equals(associationType)) {
                node.addReplyRoom(jid);
            } else if ("replyTo".equals(associationType)) {
                node.addReplyTo(jid);
            } else if ("associationTrusted".equals(associationType)) {
                ((CollectionNode) node).addAssociationTrusted(jid);
            }
        } else {
            log.warn("Node associated with JID not found " + nodeId + ". Will not be loaded.");
        }
    }

    private static void loadAssociatedGroups(Map<String, Node> nodes, DBObject nodeGroupObj) {
        String nodeId = (String) nodeGroupObj.get("nodeId");
        Node node = nodes.get(nodeId);
        if (node != null) {
            node.addAllowedRosterGroup((String) nodeGroupObj.get("rosterGroup"));
        } else {
            log.warn("Node associated with group not found " + nodeId + ". Will not be loaded.");
        }
    }

    private static void loadAffiliations(Map<String, Node> nodes, DBObject affiliationObj) {
        String nodeId = (String) affiliationObj.get("nodeId");
        Node node = nodes.get(nodeId);
        if (node != null) {
            String jid = (String) affiliationObj.get("jid");
            String affiliation = (String) affiliationObj.get("affiliation");

            NodeAffiliate affiliate = new NodeAffiliate(node, new JID(jid));
            affiliate.setAffiliation(NodeAffiliate.Affiliation.valueOf(affiliation));
            node.addAffiliate(affiliate);
        } else {
            log.warn("Node associated with affiliation not found " + nodeId + ". Will not be loaded.");
        }

    }

    private static void loadSubscriptions(Map<String, Node> nodes, DBObject subscriptionObj) {
        String nodeId = (String) subscriptionObj.get("nodeId");
        Node node = nodes.get(nodeId);
        if (node != null) {
            JID owner = new JID((String) subscriptionObj.get("owner"));
            if (node.getAffiliate(owner) != null) {
                String subId = (String) subscriptionObj.get("id");
                JID subscriber = new JID((String) subscriptionObj.get("jid"));
                String state = (String) subscriptionObj.get("state");
                Boolean deliverNotifications = (Boolean) subscriptionObj.get("deliver");
                Boolean digest = (Boolean) subscriptionObj.get("digest");
                Integer digestFrequency = (Integer) subscriptionObj.get("digestFrequency");
                Long expire = (Long) subscriptionObj.get("expire");
                Boolean includeBody = (Boolean) subscriptionObj.get("includeBody");
                String showValues = (String) subscriptionObj.get("showValues");
                String subscriptionType = (String) subscriptionObj.get("subscriptionType");
                Integer subscriptionDepth = (Integer) subscriptionObj.get("subscriptionDepth");
                String keyword = (String) subscriptionObj.get("keyword");

                NodeSubscription subscription = new NodeSubscription(node, owner, subscriber,
                        NodeSubscription.State.valueOf(state), subId);

                subscription.setShouldDeliverNotifications(deliverNotifications);
                subscription.setUsingDigest(digest);
                subscription.setDigestFrequency(digestFrequency);
                if (expire != null) {
                    subscription.setExpire(new Date(expire));
                }
                subscription.setIncludingBody(includeBody);
                subscription.setPresenceStates(decodeWithComma(showValues));
                subscription.setType(NodeSubscription.Type.valueOf(subscriptionType));
                subscription.setDepth(subscriptionDepth);
                subscription.setKeyword(keyword);

                subscription.setSavedToDB(true);
                node.addSubscription(subscription);
            } else {
                log.warn("Node associated with affiliation not found " + owner + ", " + nodeId
                        + ". Will not be loaded.");
            }

        } else {
            log.warn("Node associated with subscription not found " + nodeId + ". Will not be loaded.");
        }
    }

    private static String encodeWithComma(Collection<String> strings) {
        StringBuilder sb = new StringBuilder(90);

        for (String string : strings) {
            sb.append(string).append(",");
        }
        if (sb.length() > 0) {
            sb.setLength(sb.length() - 1);
        }

        return sb.toString();
    }

    private static Collection<String> decodeWithComma(String strings) {
        Collection<String> decoded = new ArrayList<String>();
        StringTokenizer tokenizer = new StringTokenizer(strings.trim(), ",");

        while (tokenizer.hasMoreTokens()) {
            decoded.add(tokenizer.nextToken());
        }

        return decoded;
    }

    private static DBCollection getDefaultCollection() {
        return getCollection(COLLECTION_NAME);
    }

    private static DBCollection getCollection(String collectionName) {
        DB db = UnfortunateLackOfSpringSupportFactory.getOpenfiredb();

        return db.getCollection(collectionName);
    }
}
