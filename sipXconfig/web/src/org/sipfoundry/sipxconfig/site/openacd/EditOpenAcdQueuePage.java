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

import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueue;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueueGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeAction;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition.CONDITION;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeStep;

public abstract class EditOpenAcdQueuePage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdQueuePage";
    public static final String SPACE = " ";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Bean
    public abstract SipxValidationDelegate getValidator();

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

    public abstract String getRecipe();

    public abstract void setRecipe(String recipe);

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

        getOpenAcdContext().saveQueue(queue);
        setQueueId(getQueue().getId());
        ((OpenAcdRecipeComponent) this.getComponent(OpenAcdRecipeComponent.NAME)).setRecipeSteps(null);
    }

    public boolean renderRecipe() {
        return !getQueue().isNew();
    }

    /**
     * Format recipe steps as plain English statements.
     * This method provides a "best effort" approach (for instance capitalization is completely wrong,
     * but still recipe steps are very intelligible in the English language).
     * Cannot find a reason to include this in the context, it is strictly a UI function.
     * @return
     */
    public Set<String> getInheritedRecipes() {
        Set<String> recipes = new HashSet<String>();
        Messages messages = ((OpenAcdRecipeComponent)
                this.getComponent(OpenAcdRecipeComponent.NAME)).getComponent("recipeCondition").getMessages();
        for (OpenAcdRecipeStep step : getQueue().getGroup().getSteps()) {
            StringBuilder criteria = new StringBuilder(messages.getMessage("if"));
            criteria.append(SPACE);
            Iterator<OpenAcdRecipeCondition> conditionIterator = step.getConditions().iterator();
            while (conditionIterator.hasNext()) {
                OpenAcdRecipeCondition condition = conditionIterator.next();
                criteria.append(messages.getMessage(condition.getCondition()));
                criteria.append(SPACE);
                criteria.append(messages.getMessage(condition.getRelation()));
                criteria.append(SPACE);
                if (condition.getCondition().equals(CONDITION.DAY_OF_WEEK.toString())) {
                    criteria.append(getDayOfWeek(new Integer(condition.getValueCondition()), messages));
                } else if (condition.getCondition().equals(CONDITION.MEDIA_TYPE.toString())) {
                    criteria.append(messages.getMessage(condition.getValueCondition()));
                } else {
                    criteria.append(condition.getValueCondition());
                }
                if (conditionIterator.hasNext()) {
                    criteria.append(SPACE);
                    criteria.append(messages.getMessage("and"));
                }
                criteria.append(SPACE);
            }
            StringBuilder recipeBuilder = new StringBuilder(criteria);
            recipeBuilder.append(messages.getMessage("then"));
            recipeBuilder.append(SPACE);
            recipeBuilder.append(((OpenAcdRecipeComponent) this.getComponent(OpenAcdRecipeComponent.NAME))
                    .getMessages().getMessage(step.getAction().getAction()));
            if (step.getAction().getAction().equals(OpenAcdRecipeAction.ACTION.ADD_SKILLS.toString())
                    || step.getAction().getAction().equals(OpenAcdRecipeAction.ACTION.REMOVE_SKILLS.toString())) {
                recipeBuilder.append(": ");
                recipeBuilder.append(StringUtils.join(step.getAction().getAllSkillNames(), ", "));
            } else if (step.getAction().getActionValue() != null) {
                recipeBuilder.append(SPACE);
                recipeBuilder.append(step.getAction().getActionValue());
            }
            recipeBuilder.append("; ");
            recipeBuilder.append(((OpenAcdRecipeComponent) this.getComponent(OpenAcdRecipeComponent.NAME))
                    .getMessages().getMessage(step.getFrequency()));
            recipes.add(recipeBuilder.toString());
        }

        return recipes;
    }

    private String getDayOfWeek(int position, Messages messages) {
        switch (position) {
        case 1:
            return messages.getMessage("sunday");
        case 2:
            return messages.getMessage("monday");
        case 3:
            return messages.getMessage("tuesday");
        case 4:
            return messages.getMessage("wednesday");
        case 5:
            return messages.getMessage("thursday");
        case 6:
            return messages.getMessage("friday");
        case 7:
            return messages.getMessage("saturday");
        default:
            return "";
        }
    }
}
