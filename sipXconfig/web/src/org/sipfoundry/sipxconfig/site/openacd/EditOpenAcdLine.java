/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.openacd;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.BooleanUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdExtension;
import org.sipfoundry.sipxconfig.service.SipxOpenAcdService;

public abstract class EditOpenAcdLine extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdLine";
    private static final String SLASH = "/";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject("spring:sipxOpenAcdService")
    public abstract SipxOpenAcdService getSipxOpenAcdService();

    public abstract String getName();

    public abstract void setName(String name);

    public abstract String getDescription();

    public abstract void setDescription(String description);

    public abstract String getLineNumber();

    public abstract void setLineNumber(String number);

    public abstract String getQueue();

    public abstract void setQueue(String queue);

    @Persist
    public abstract String getWelcomeMessage();

    public abstract void setWelcomeMessage(String path);

    public abstract boolean isAllowVoicemail();

    public abstract void setAllowVoicemail(boolean allow);

    public abstract boolean isAnswerSupervision();

    public abstract void setAnswerSupervision(boolean answer);

    public abstract ActionBean getActionBean();

    public abstract void setActionBean(ActionBean a);

    public abstract int getIndex();

    public abstract void setIndex(int i);

    @Persist
    public abstract Location getSipxLocation();

    public abstract void setSipxLocation(Location locationId);

    @Persist
    public abstract Integer getOpenAcdLineId();

    public abstract void setOpenAcdLineId(Integer id);

    public abstract String getFilter();

    public abstract void setFilter(String filter);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract ActionBean getRemoveAction();

    public abstract void setRemoveAction(ActionBean bean);

    @Persist
    public abstract Collection<ActionBean> getActions();

    public abstract void setActions(Collection<ActionBean> actions);

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageEndRender(event);
        List<FreeswitchAction> actions = null;

        if (getOpenAcdLineId() == null) {
            actions = OpenAcdExtension.getDefaultActions(getSipxLocation());
        } else {
            OpenAcdExtension line = getOpenAcdContext().getExtensionById(getOpenAcdLineId());
            actions = line.getLineActions();
            setName(line.getName());
            setDescription(line.getDescription());
            setLineNumber(line.getLineCondition().getLineNumber());
        }

        List<ActionBean> actionBeans = new LinkedList<ActionBean>();
        for (FreeswitchAction action : actions) {
            String application = action.getApplication();
            String data = action.getData();
            if (StringUtils.equals(application, FreeswitchAction.PredefinedAction.answer.toString())) {
                setAnswerSupervision(true);
            } else if (StringUtils.contains(data, OpenAcdExtension.Q)) {
                if (getQueue() == null) {
                    setQueue(StringUtils.removeStart(data, OpenAcdExtension.Q));
                }
            } else if (StringUtils.contains(data, OpenAcdExtension.ALLOW_VOICEMAIL)) {
                setAllowVoicemail(BooleanUtils.toBoolean(StringUtils.removeStart(data,
                        OpenAcdExtension.ALLOW_VOICEMAIL)));
            } else if (StringUtils.equals(application, FreeswitchAction.PredefinedAction.playback.toString())) {
                if (getWelcomeMessage() == null) {
                    setWelcomeMessage(StringUtils.removeStart(data, getSipxOpenAcdService().getAudioDir() + SLASH));
                }
            } else {
                actionBeans.add(new ActionBean(action));
            }
        }
        if (getActions() == null) {
            setActions(actionBeans);
        }
        if (getRemoveAction() != null) {
            getActions().remove(getRemoveAction());
        }
    }

    public void addAction() {
        FreeswitchAction action = new FreeswitchAction();
        ActionBean bean = new ActionBean(action);
        getActions().add(bean);
    }

    public String[] getOpenAcdApplicationNames() {
        String filter = getFilter();

        if (filter == null || filter.length() < 1) {
            return getOpenAcdContext().getOpenAcdApplicationNames();
        }
        List<String> temp = new ArrayList<String>();
        for (String app : getOpenAcdContext().getOpenAcdApplicationNames()) {
            if (app.startsWith(filter)) {
                temp.add(app);
            }
        }
        return temp.toArray(new String[0]);
    }

    public void filterList(String filter) {
        setFilter(filter);
    }

    public void saveLine() {
        // save the line and reload
        if (TapestryUtils.isValid(this)) {
            OpenAcdExtension line = null;
            if (getOpenAcdLineId() != null) {
                line = getOpenAcdContext().getExtensionById(getOpenAcdLineId());
            } else {
                line = new OpenAcdExtension();
                line.addCondition(OpenAcdExtension.createLineCondition());
            }

            line.setName(getName());
            line.setDescription(getDescription());
            line.setLocation(getSipxLocation());

            // add common actions
            line.getLineCondition().getActions().clear();
            line.getLineCondition().addAction(OpenAcdExtension.createAnswerAction(isAnswerSupervision()));
            line.getLineCondition().addAction(OpenAcdExtension.createVoicemailAction(isAllowVoicemail()));
            line.getLineCondition().addAction(OpenAcdExtension.createQueueAction(getQueue()));
            line.getLineCondition().addAction(
                    OpenAcdExtension.createPlaybackAction(getSipxOpenAcdService().getAudioDir() + SLASH
                            + getWelcomeMessage()));

            for (ActionBean actionBean : getActions()) {
                line.getLineCondition().addAction((FreeswitchAction) actionBean.getAction().duplicate());
            }

            line.getLineCondition().setExpression(
                    String.format(OpenAcdExtension.DESTINATION_NUMBER_PATTERN, getLineNumber()));
            getOpenAcdContext().saveExtension(line);
            setOpenAcdLineId(line.getId());
            setActions(null);
            setWelcomeMessage(null);
        }
    }

}
