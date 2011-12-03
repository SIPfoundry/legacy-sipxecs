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
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.components.selection.AdaptedSelectionModel;
import org.sipfoundry.sipxconfig.components.selection.OptGroup;
import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgent;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdAgentsPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SelectMap getSelections();

    public abstract void setAgents(List<OpenAcdAgent> agents);

    public abstract List<OpenAcdAgent> getAgents();

    public abstract void setCurrentRow(OpenAcdAgent agent);

    public abstract Collection getSelectedRows();

    public IPage addAgents(IRequestCycle cycle) {
        OpenAcdAddAgentsToGroupPage page = (OpenAcdAddAgentsToGroupPage) cycle
                .getPage(OpenAcdAddAgentsToGroupPage.PAGE);
        page.addAgentsToGroup(getPage().getPageName());
        return page;
    }

    public IPage editAgent(IRequestCycle cycle, Integer agentId) {
        EditOpenAcdAgentPage page = (EditOpenAcdAgentPage) cycle.getPage(EditOpenAcdAgentPage.PAGE);
        page.editAgent(agentId, getPage().getPageName());
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        setAgents(getOpenAcdContext().getAgents());
    }

    public void delete() {
        Collection<Integer> ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }
        try {
            for (Integer id : ids) {
                OpenAcdAgent agent = getOpenAcdContext().getAgentById(id);
                getOpenAcdContext().deleteAgent(agent);
            }
        } catch (UserException ex) {
            //forward the error message display to contained page for this component
            //components do not scale very well on UserException automatic catch
            SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(getPage());
            validator.record(ex, getMessages());
        }
    }

    public IPropertySelectionModel getActionModel() {
        Collection<OptionAdapter> actions = new ArrayList<OptionAdapter>();
        Collection<OpenAcdAgentGroup> groups = getOpenAcdContext().getAgentGroups();

        if (!groups.isEmpty()) {
            actions.add(new OptGroup(getMessages().getMessage("label.moveToAgentGroup")));

            for (OpenAcdAgentGroup group : groups) {
                actions.add(new MoveToAgentGroupAction(group));
            }
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);

        return model;
    }

    class MoveToAgentGroupAction extends BulkGroupAction {

        private final OpenAcdAgentGroup m_group;

        MoveToAgentGroupAction(OpenAcdAgentGroup group) {
            super(null);
            m_group = group;
        }

        @Override
        public String getLabel(Object option, int index) {
            return m_group.getName();
        }

        @Override
        public Object getValue(Object option, int index) {
            return m_group.getId();
        }

        @Override
        public String squeezeOption(Object option, int index) {
            return m_group.getId().toString();
        }

        @Override
        public void actionTriggered(IComponent component, IRequestCycle cycle) {
            Collection<Integer> ids = getSelections().getAllSelected();
            if (ids.isEmpty()) {
                return;
            }
            try {
                for (Integer id : ids) {
                    OpenAcdAgent agent = getOpenAcdContext().getAgentById(id);
                    agent.setGroup(m_group);
                    getOpenAcdContext().saveAgent(agent);
                }
            } catch (UserException ex) {
                IValidationDelegate validator = TapestryUtils.getValidator(getPage());
                validator.record(new ValidatorException(getMessages().getMessage("msg.cannot.connect")));
            }
        }
    }
}
