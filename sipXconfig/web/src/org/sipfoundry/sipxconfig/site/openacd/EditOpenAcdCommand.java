/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.openacd;

import java.util.ArrayList;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.openacd.OpenAcdCommand;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdExtension;

public abstract class EditOpenAcdCommand extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdCommand";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    public abstract boolean isEnabled();

    public abstract void setEnabled(boolean enabled);

    public abstract String getName();

    public abstract void setName(String name);

    public abstract String getDescription();

    public abstract void setDescription(String description);

    public abstract String getLineNumber();

    public abstract void setLineNumber(String number);

    public abstract ActionBean getActionBean();

    public abstract void setActionBean(ActionBean a);

    public abstract int getIndex();

    public abstract void setIndex(int i);

    @Persist
    public abstract Integer getOpenAcdCommandId();

    public abstract void setOpenAcdCommandId(Integer id);

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

        if (getOpenAcdCommandId() == null) {
            actions = OpenAcdCommand.getDefaultActions(getLocationsManager().getPrimaryLocation());
        } else {
            OpenAcdCommand line = (OpenAcdCommand) getOpenAcdContext().getExtensionById(getOpenAcdCommandId());
            actions = line.getLineActions();
            setName(line.getName());
            setEnabled(line.isEnabled());
            setDescription(line.getDescription());
            setLineNumber(line.getNumberCondition().getExtension());
        }

        List<ActionBean> actionBeans = new LinkedList<ActionBean>();
        for (FreeswitchAction action : actions) {
            String application = action.getApplication();
            if (StringUtils.equals(application, FreeswitchAction.PredefinedAction.answer.toString())) {
                continue;
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

    public void saveCommand() {
        // save the line and reload
        if (TapestryUtils.isValid(this)) {
            OpenAcdCommand cmd = null;
            if (getOpenAcdCommandId() != null) {
                cmd = (OpenAcdCommand) getOpenAcdContext().getExtensionById(getOpenAcdCommandId());
            } else {
                cmd = getOpenAcdContext().newOpenAcdCommand();
                cmd.addCondition(OpenAcdCommand.createLineCondition());
            }

            cmd.setName(getName());
            cmd.setEnabled(isEnabled());
            cmd.setDescription(getDescription());

            // add common actions
            cmd.getNumberCondition().getActions().clear();
            cmd.getNumberCondition().addAction(
                    OpenAcdCommand.createAction(FreeswitchAction.PredefinedAction.answer.toString(), null));

            for (ActionBean actionBean : getActions()) {
                cmd.getNumberCondition().addAction((FreeswitchAction) actionBean.getAction().duplicate());
            }

            cmd.getNumberCondition().setExpression(
                    String.format(OpenAcdExtension.DESTINATION_NUMBER_PATTERN, getLineNumber()));
            getOpenAcdContext().saveExtension(cmd);
            setOpenAcdCommandId(cmd.getId());
            setActions(null);
        }
    }

}
