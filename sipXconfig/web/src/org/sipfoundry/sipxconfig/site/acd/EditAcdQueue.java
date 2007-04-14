/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import java.io.Serializable;
import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditAcdQueue extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "EditAcdQueue";

    public abstract AcdContext getAcdContext();

    public abstract Serializable getAcdServerId();

    public abstract void setAcdServerId(Serializable id);

    public abstract AcdQueue getAcdQueue();

    public abstract void setAcdQueue(AcdQueue acdQueue);

    public abstract Serializable getAcdQueueId();

    public abstract void setAcdQueueId(Serializable acdQueueId);

    public abstract Serializable getOverflowQueueId();

    public abstract void setOverflowQueueId(Serializable acdQueueId);

    public abstract boolean getChanged();

    public abstract void setQueuesModel(ObjectSelectionModel queuesModel);

    public abstract void setTab(String tab);
    
    public void pageBeginRender(PageEvent event_) {
        AcdQueue acdQueue = getAcdQueue();
        if (acdQueue != null) {
            return;
        }
        AcdContext acdContext = getAcdContext();
        Serializable id = getAcdQueueId();
        if (id != null) {
            acdQueue = acdContext.loadQueue(id);
            AcdServer acdServer = acdQueue.getAcdServer();
            setAcdServerId(acdServer.getId());
        } else {
            acdQueue = acdContext.newQueue();
            setTab("config");
        }
        setAcdQueue(acdQueue);
        AcdQueue overflowQueue = acdQueue.getOverflowQueue();
        if (overflowQueue != null) {
            setOverflowQueueId(overflowQueue.getId());
        }
        AcdServer acdServer = getAcdContext().loadServer(getAcdServerId());
        setQueuesModel(new AcdQueueSelectionModel(acdServer, acdQueue));
    }

    public void formSubmit() {
        if (getChanged()) {
            // make sure that queue is fetched from DB on render
            setAcdQueue(null);
        }
    }

    public void apply() {
        if (TapestryUtils.isValid(this)) {
            saveValid();
        }
    }

    private void saveValid() {
        AcdContext acdContext = getAcdContext();
        AcdQueue acdQueue = getAcdQueue();
        if (acdQueue.isNew()) {
            Serializable serverId = getAcdServerId();
            AcdServer server = acdContext.loadServer(serverId);
            server.insertQueue(acdQueue);
        }
        Serializable overflowQueueId = getOverflowQueueId();
        if (overflowQueueId == null) {
            acdQueue.setOverflowQueue(null);
        } else {
            AcdQueue overflowQueue = acdContext.loadQueue(overflowQueueId);
            acdQueue.setOverflowQueue(overflowQueue);
        }
        acdContext.store(acdQueue);
        setAcdQueueId(acdQueue.getId());
    }

    public IPage addAgent(IRequestCycle cycle) {
        AddAcdAgent addPage = (AddAcdAgent) cycle.getPage(AddAcdAgent.PAGE);
        addPage.setAcdQueueId(getAcdQueueId());
        addPage.setReturnPage(this);
        return addPage;
    }

    public IPage editAgent(IRequestCycle cycle, Integer id) {
        EditAcdAgent editPage = (EditAcdAgent) cycle.getPage(EditAcdAgent.PAGE);
        editPage.setAcdAgentId(id);
        editPage.setReturnPage(this);
        return editPage;
    }
    
    public String getAcdQueueUri() {
        AcdQueue q = getAcdQueue();
        if (q == null) {
            return null;
        }
        
        return q.calculateUri();
    }

    public static class AcdQueueSelectionModel extends ObjectSelectionModel {
        AcdQueueSelectionModel(AcdServer acdServer, AcdQueue excludeQueue) {
            Collection queues = acdServer.getQueues();
            queues.remove(excludeQueue);
            setCollection(queues);
            setLabelExpression("name");
            setValueExpression("id");
        }
    }
}
