/**
 *
 *
 * Copyright (c) 2010 / 2012 eZuce, Inc. All rights reserved.
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
import java.util.Map;
import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.NamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdObjectWithRecipe;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeAction.ACTION;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeStep;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeStep.FREQUENCY;
import org.sipfoundry.sipxconfig.openacd.RecipeStepBean;

public abstract class OpenAcdRecipeComponent extends BaseComponent implements PageBeginRenderListener {

    public static final String NAME = "recipes";
    public abstract RecipeStepBean getRecipeStepBean();

    public abstract void setRecipeStepBean(RecipeStepBean bean);

    @Persist
    public abstract Map<Integer, RecipeStepBean> getRecipeSteps();

    public abstract void setRecipeSteps(Map<Integer, RecipeStepBean> steps);

    @Parameter(required = true)
    public abstract OpenAcdObjectWithRecipe getObjectWithRecipe();

    public abstract void setObjectWithRecipe(OpenAcdObjectWithRecipe q);

    @Persist
    public abstract Integer getLastPosition();

    public abstract void setLastPosition(Integer position);

    @Bean
    public abstract SelectMap getSelections();
    public abstract Collection getSelectedRows();

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getRecipeSteps() == null) {
            Map<Integer, RecipeStepBean> stepBeans = new LinkedHashMap<Integer, RecipeStepBean>();
            Set<OpenAcdRecipeStep> steps = getObjectWithRecipe().getSteps();
            int i = 0;
            for (OpenAcdRecipeStep step : steps) {
                stepBeans.put(i, new RecipeStepBean(step, i));
                i++;
            }
            setRecipeSteps(stepBeans);
            setLastPosition(i);
        }
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            Set<OpenAcdRecipeStep> steps = new LinkedHashSet<OpenAcdRecipeStep>();
            for (RecipeStepBean bean : getRecipeSteps().values()) {
                steps.add(bean.getRecipeStep());
            }
            getObjectWithRecipe().setSteps(steps);
        }
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

    public void deleteSteps() {
        Collection<RecipeStepBean> beans = getSelectedRows();
        if (beans.isEmpty()) {
            return;
        }
        for (RecipeStepBean bean : beans) {
            getRecipeSteps().remove(bean.getPosition());
        }
    }

    public IPropertySelectionModel getActionModel() {
        Map<String, String> types2Labels = new LinkedHashMap<String, String>();
        for (ACTION value : ACTION.values()) {
            types2Labels.put(value.toString(), getMessages().getMessage(value.toString()));
        }
        return new NamedValuesSelectionModel(types2Labels);
    }

    public String getActionHelpText() {
        return getMessages()
                .getMessage("description." + getRecipeStepBean().getRecipeStep().getAction().getAction());
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

    public boolean isTransferOutband() {
        String actionValue = getRecipeStepBean().getRecipeStep().getAction().getAction();
        return actionValue.equals(ACTION.TRANSFER_OUTBAND.toString());
    }
}
