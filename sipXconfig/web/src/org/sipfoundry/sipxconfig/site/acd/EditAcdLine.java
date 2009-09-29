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

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdLine;
import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.acd.EditAcdQueue.AcdQueueSelectionModel;

public abstract class EditAcdLine extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "acd/EditAcdLine";

    public abstract AcdContext getAcdContext();

    public abstract Serializable getAcdServerId();

    public abstract void setAcdServerId(Serializable id);

    public abstract AcdLine getAcdLine();

    public abstract void setAcdLine(AcdLine acdLine);

    public abstract Serializable getAcdLineId();

    public abstract void setAcdLineId(Serializable acdLineId);

    public abstract Serializable getAcdQueueId();

    public abstract void setAcdQueueId(Serializable acdQueueId);

    public abstract void setQueuesModel(ObjectSelectionModel queuesModel);

    public void pageBeginRender(PageEvent event_) {
        AcdLine acdLine = getAcdLine();
        if (acdLine != null) {
            return;
        }
        AcdContext acdContext = getAcdContext();
        Serializable id = getAcdLineId();
        if (id == null) {
            acdLine = acdContext.newLine();
        } else {
            acdLine = acdContext.loadLine(id);
            AcdServer server = acdLine.getAcdServer();
            setAcdServerId(server.getId());
            AcdQueue queue = acdLine.getAcdQueue();
            if (queue != null) {
                setAcdQueueId(queue.getId());
            }
        }
        AcdServer acdServer = getAcdContext().loadServer(getAcdServerId());
        setQueuesModel(new AcdQueueSelectionModel(acdServer, null));
        setAcdLine(acdLine);
    }

    public void apply() {
        if (TapestryUtils.isValid(this)) {
            saveValid();
        }
    }

    void saveValid() {
        AcdContext acdContext = getAcdContext();
        AcdLine acdLine = getAcdLine();
        if (acdLine.isNew()) {
            AcdServer server = acdContext.loadServer(getAcdServerId());
            server.insertLine(acdLine);
        }
        acdContext.store(acdLine);
        acdContext.associate(acdLine.getId(), getAcdQueueId());
        setAcdLineId(acdLine.getId());
    }
}
