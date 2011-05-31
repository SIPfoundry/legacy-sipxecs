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
import java.util.List;

import org.apache.commons.lang.StringUtils;
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
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueue;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueueGroup;
import org.sipfoundry.sipxconfig.site.setting.BulkGroupAction;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdQueuesPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract List<OpenAcdQueue> getQueues();

    public abstract void setQueues(List<OpenAcdQueue> queues);

    public abstract Collection getSelectedRows();

    public abstract void setCurrentRow(OpenAcdQueue queue);

    public IPage addQueue(IRequestCycle cycle) {
        EditOpenAcdQueuePage page = (EditOpenAcdQueuePage) cycle.getPage(EditOpenAcdQueuePage.PAGE);
        page.addQueue(getPage().getPageName());
        return page;
    }

    public IPage editQueue(IRequestCycle cycle, Integer queueId) {
        EditOpenAcdQueuePage page = (EditOpenAcdQueuePage) cycle.getPage(EditOpenAcdQueuePage.PAGE);
        page.editQueue(queueId, getPage().getPageName());
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        List<OpenAcdQueue> queues = getOpenAcdContext().getQueues();
        setQueues(queues);
    }

    public void delete() {
        Collection ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }
        List<String> queues = getOpenAcdContext().removeQueues(ids);
        if (!queues.isEmpty()) {
            String queueNames = StringUtils.join(queues.iterator(), ", ");
            String errMessage = getMessages().format("msg.err.queueDeletion", queueNames);
            IValidationDelegate validator = TapestryUtils.getValidator(getPage());
            validator.record(new ValidatorException(errMessage));
        }
    }

    public IPropertySelectionModel getActionModel() {
        Collection<OptionAdapter> actions = new ArrayList<OptionAdapter>();
        Collection<OpenAcdQueueGroup> groups = getOpenAcdContext().getQueueGroups();

        if (!groups.isEmpty()) {
            actions.add(new OptGroup(getMessages().getMessage("label.moveToQueueGroup")));

            for (OpenAcdQueueGroup group : groups) {
                actions.add(new AddToQueueGroupAction(group));
            }
        }

        AdaptedSelectionModel model = new AdaptedSelectionModel();
        model.setCollection(actions);

        return model;
    }

    class AddToQueueGroupAction extends BulkGroupAction {

        private final OpenAcdQueueGroup m_group;

        AddToQueueGroupAction(OpenAcdQueueGroup group) {
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

        public void actionTriggered(IComponent component, IRequestCycle cycle) {
            Collection<Integer> ids = getSelections().getAllSelected();
            if (ids.isEmpty()) {
                return;
            }
            try {
                for (Integer id : ids) {
                    OpenAcdQueue queue = getOpenAcdContext().getQueueById(id);
                    queue.setGroup(m_group);
                    getOpenAcdContext().saveQueue(queue);
                }
            } catch (UserException ex) {
                IValidationDelegate validator = TapestryUtils.getValidator(getPage());
                validator.record(new ValidatorException(getMessages().getMessage("msg.cannot.connect")));
            }
        }
    }
}
