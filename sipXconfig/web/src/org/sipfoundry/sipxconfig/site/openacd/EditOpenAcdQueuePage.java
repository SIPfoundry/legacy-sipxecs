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

import java.util.Collection;
import java.util.LinkedHashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.NamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueue;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueueGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeStep;
import org.sipfoundry.sipxconfig.openacd.RecipeStepBean;

import static org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeAction.ACTION;
import static org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeStep.FREQUENCY;

public abstract class EditOpenAcdQueuePage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdQueuePage";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @Persist
    public abstract Integer getQueueId();

    public abstract void setQueueId(Integer queueId);

    @Persist
    public abstract OpenAcdQueue getQueue();

    public abstract void setQueue(OpenAcdQueue queue);

    public abstract void setSelectedQueueGroup(OpenAcdQueueGroup selectedQueueGroup);

    public abstract OpenAcdQueueGroup getSelectedQueueGroup();

    public abstract List<String> getInheritedSkills();

    public abstract void setInheritedSkills(List<String> skills);

    public abstract String getSkill();

    public abstract void setSkill(String skill);

    @Persist
    public abstract Map<Integer, RecipeStepBean> getRecipeSteps();

    public abstract void setRecipeSteps(Map<Integer, RecipeStepBean> steps);

    public abstract RecipeStepBean getRecipeStepBean();

    public abstract void setRecipeStepBean(RecipeStepBean bean);

    public abstract Collection getSelectedRows();

    @Persist
    public abstract Integer getLastPosition();

    public abstract void setLastPosition(Integer position);

    public void addQueue(String returnPage) {
        setQueueId(null);
        setQueue(null);
        setReturnPage(returnPage);
    }

    public void editQueue(Integer queueId, String returnPage) {
        setQueueId(queueId);
        setReturnPage(returnPage);
    }

    public IPage addSkills(IRequestCycle cycle) {
        OpenAcdServerPage page = (OpenAcdServerPage) cycle.getPage(OpenAcdServerPage.PAGE);
        page.setTab("skills");
        return page;
    }

    public void addRecipeStep() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        OpenAcdRecipeStep step = new OpenAcdRecipeStep();
        RecipeStepBean bean = new RecipeStepBean(step, getLastPosition());
        getRecipeSteps().put(getLastPosition(), bean);
        setLastPosition(getLastPosition() + 1);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getQueueId() == null) {
            setQueue(new OpenAcdQueue());
        } else {
            OpenAcdQueue queue = getOpenAcdContext().getQueueById(getQueueId());
            setQueue(queue);
            OpenAcdQueueGroup queueGroup = queue.getGroup();
            if (queueGroup != null) {
                setSelectedQueueGroup(queueGroup);
                setInheritedSkills(queueGroup.getAllSkillNames());
            }
        }
        if (getRecipeSteps() == null) {
            Map<Integer, RecipeStepBean> stepBeans = new LinkedHashMap<Integer, RecipeStepBean>();
            Set<OpenAcdRecipeStep> steps = getQueue().getSteps();
            int i = 0;
            for (OpenAcdRecipeStep step : steps) {
                stepBeans.put(i, new RecipeStepBean(step, i));
                i++;
            }
            setRecipeSteps(stepBeans);
            setLastPosition(i);
        }
    }

    public void deleteSteps() {
        Collection<RecipeStepBean> beans = getSelectedRows();
        if (beans.isEmpty()) {
            return;
        }
        for (RecipeStepBean bean : beans) {
            getRecipeSteps().remove(bean.getPosition());
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        OpenAcdQueue queue = getQueue();
        OpenAcdQueueGroup selectedQueueGroup = getSelectedQueueGroup();
        if (selectedQueueGroup == null) {
            throw new UserException(getMessages().getMessage("error.requiredQueueGroup"));
        }
        queue.setGroup(selectedQueueGroup);
        Set<OpenAcdRecipeStep> steps = new LinkedHashSet<OpenAcdRecipeStep>();
        for (RecipeStepBean bean : getRecipeSteps().values()) {
            steps.add(bean.getRecipeStep());
        }
        queue.setSteps(steps);

        getOpenAcdContext().saveQueue(queue);
        setQueueId(getQueue().getId());
        setRecipeSteps(null);
    }

    public IPropertySelectionModel getActionModel() {
        Map<String, String> types2Labels = new LinkedHashMap<String, String>();
        for (ACTION value : ACTION.values()) {
            types2Labels.put(value.toString(), getMessages().getMessage(value.toString()));
        }
        return new NamedValuesSelectionModel(types2Labels);
    }

    public String getActionHelpText() {
        return getMessages().getMessage("description." + getRecipeStepBean().getRecipeStep().getAction().getAction());
    }

    public IPropertySelectionModel getFrequencyModel() {
        Map<String, String> types2Labels = new LinkedHashMap<String, String>();
        for (FREQUENCY value : FREQUENCY.values()) {
            types2Labels.put(value.toString(), getMessages().getMessage(value.toString()));
        }
        return new NamedValuesSelectionModel(types2Labels);
    }

    public boolean isSkillComponent() {
        String actionValue = getRecipeStepBean().getRecipeStep().getAction().getAction();
        return actionValue.equals(ACTION.ADD_SKILLS.toString())
                || actionValue.equals(ACTION.REMOVE_SKILLS.toString());
    }

    public boolean isPriorityComponent() {
        String actionValue = getRecipeStepBean().getRecipeStep().getAction().getAction();
        return actionValue.equals(ACTION.SET_PRIORITY.toString());
    }

    public boolean isMediaAnnounceComponent() {
        String actionValue = getRecipeStepBean().getRecipeStep().getAction().getAction();
        return actionValue.equals(ACTION.MEDIA_ANNOUCE.toString());
    }

    public boolean renderRecipe() {
        return !getQueue().isNew();
    }
}
