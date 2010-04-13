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
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.acd.stats.AcdStatistics;
import org.sipfoundry.sipxconfig.acd.stats.AcdStatisticsImpl;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.setting.AbstractSetting;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;

public abstract class EditAcdQueue extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "acd/EditAcdQueue";

    private static final String ID = "id";

    private static final String NAME = "name";

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

    public abstract void setTab(String tab);

    public abstract AcdQueueSelectionModel getQueuesModel();

    public abstract AcdHuntGroupsSelectionModel getHuntGroupModel();

    public abstract void setQueuesModel(AcdQueueSelectionModel queuesModel);

    public abstract void setHuntGroupModel(AcdHuntGroupsSelectionModel huntGroupModel);

    public abstract Setting getOverflowTypeSetting();

    public abstract void setOverflowTypeSetting(Setting overflowTypeSetting);

    public abstract Setting getOverflowTypeValueSetting();

    public abstract void setOverflowTypeValueSetting(Setting overflowTypeValueSetting);

    public abstract AcdStatistics getAcdStatistics();

    public abstract void setAcdStatistics(AcdStatistics stats);

    @InjectObject(value = "spring:callGroupContext")
    public abstract CallGroupContext getCallGroupContext();

    public void pageBeginRender(PageEvent event_) {
        AcdQueue acdQueue = getAcdQueue();

        if (acdQueue != null) {
            return;
        }

        AcdContext acdContext = getAcdContext();

        if (getAcdStatistics() == null) {
            setAcdStatistics(new AcdStatisticsImpl(acdContext));
        }

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

        setOverflowTypeSetting(getAcdQueue().getOverflowType());
        setOverflowTypeValueSetting(getAcdQueue().getOverflowTypeValue());

        EnumSetting enumType = (EnumSetting) getOverflowTypeSetting().getType();
        enumType.setListenOnChange(true);
        enumType.setPromptSelect(true);

        EnumSetting enumTypeValue = (EnumSetting) getOverflowTypeValueSetting().getType();
        enumTypeValue.setPromptSelect(true);

        AcdServer acdServer = getAcdContext().loadServer(getAcdServerId());
        setQueuesModel(new AcdQueueSelectionModel(acdServer, getAcdQueue()));
        setHuntGroupModel(new AcdHuntGroupsSelectionModel(getCallGroupContext()));

        if (!event_.getRequestCycle().isRewinding()) {
            refreshOverflowSettings();
        }
    }

    public void formSubmit() {
        if (getChanged()) {
            // make sure that queue is fetched from DB on render
            setAcdQueue(null);
        } else {
            refreshOverflowSettings();
        }
    }

    private void refreshOverflowSettings() {
        Setting overflowType = getOverflowTypeSetting();
        Setting overflowValue = getOverflowTypeValueSetting();
        EnumSetting overflowValueType = (EnumSetting) overflowValue.getType();
        overflowValueType.clearEnums();

        String value = overflowType.getValue();
        if (value != null) {
            if (value.equals(AcdQueue.QUEUE_TYPE)) {
                if (getQueuesModel().getOptionCount() == 0) {
                    enableOverflowEntry(true);
                    overflowValueType.setPromptSelect(true);
                } else {
                    for (int i = 0; i < getQueuesModel().getOptionCount(); i++) {
                        overflowValueType.addEnum(getQueuesModel().getOption(i).toString(), getQueuesModel()
                                .getLabel(i));
                    }
                    enableOverflowEntry(false);
                    overflowValueType.setPromptSelect(false);
                }
            } else if (value.equals(AcdQueue.HUNTGROUP_TYPE)) {
                if (getHuntGroupModel().getOptionCount() == 0) {
                    enableOverflowEntry(true);
                    overflowValueType.setPromptSelect(true);
                } else {
                    for (int i = 0; i < getHuntGroupModel().getOptionCount(); i++) {
                        overflowValueType.addEnum(getHuntGroupModel().getOption(i).toString(), getHuntGroupModel()
                                .getLabel(i));
                    }
                    enableOverflowEntry(false);
                    overflowValueType.setPromptSelect(false);
                }
            }
        } else {
            overflowValueType.setPromptSelect(true);
            enableOverflowEntry(true);
        }
    }

    private void enableOverflowEntry(boolean enable) {
        if (enable) {
            ((AbstractSetting) getAcdQueue().getOverflowEntry()).setEnabled(true);
        } else {
            ((AbstractSetting) getAcdQueue().getOverflowEntry()).setValue("");
            ((AbstractSetting) getAcdQueue().getOverflowEntry()).setEnabled(false);
        }

    }

    public void apply() {
        if (TapestryUtils.isValid(this)) {
            if (getOverflowTypeSetting().getValue() != null && getOverflowTypeValueSetting().getValue() != null
                    && getOverflowTypeSetting().getValue().equals(AcdQueue.QUEUE_TYPE)) {
                Integer overflowQueueId = Integer.valueOf(getOverflowTypeValueSetting().getValue()).intValue();
                setOverflowQueueId(overflowQueueId);
            } else {
                if (getOverflowTypeValueSetting().getValue() == null) {
                    getOverflowTypeSetting().setValue(null);
                }
                setOverflowQueueId(null);
            }
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
        if (q == null || q.getAcdServer() == null) {
            return null;
        }

        return q.calculateUri();
    }

    public static class AcdQueueSelectionModel extends ObjectSelectionModel {
        AcdQueueSelectionModel(AcdServer acdServer, AcdQueue excludeQueue) {
            Collection queues = acdServer.getQueues();
            queues.remove(excludeQueue);
            setCollection(queues);
            setLabelExpression(NAME);
            setValueExpression(ID);
        }
    }

    public static class AcdHuntGroupsSelectionModel extends ObjectSelectionModel {
        AcdHuntGroupsSelectionModel(CallGroupContext callGroupContext) {
            Collection huntGroups = callGroupContext.getCallGroups();
            setCollection(huntGroups);
            setLabelExpression(NAME);
            setValueExpression(ID);
        }
    }
}
