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

import java.util.LinkedHashMap;
import java.util.Map;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.NamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.components.SafeNamedValuesSelectionModel;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition.CONDITION;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition.RELATION;
import org.sipfoundry.sipxconfig.openacd.RecipeStepBean;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdRecipeConditionComponent extends BaseComponent {

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Parameter
    public abstract RecipeStepBean getRecipeStepBean();

    public abstract void setRecipeStepBean(RecipeStepBean step);

    public abstract int getIndex();

    @Persist
    public abstract Integer getAddIndex();

    public abstract void setAddIndex(Integer index);

    @Persist
    public abstract Integer getRemoveIndex();

    public abstract void setRemoveIndex(Integer index);

    public abstract OpenAcdRecipeCondition getRecipeCondition();

    public abstract void setRecipeCondition(OpenAcdRecipeCondition recipeCondition);

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            if (TapestryUtils.isValid(cycle, this) && getAddIndex() != null
                    && getAddIndex() == getRecipeStepBean().getPosition()) {
                if (getRecipeStepBean().getRecipeStep().getConditions() != null) {
                    getRecipeStepBean().getRecipeStep().getConditions().add(new OpenAcdRecipeCondition());
                }
                setAddIndex(null);
            }
            if (getRemoveIndex() != null) {
                OpenAcdRecipeCondition cond = getRecipeStepBean().getRecipeStep().getConditions()
                        .get(getRemoveIndex());
                getRecipeStepBean().getRecipeStep().removeCondition(cond);
                setRemoveIndex(null);
            }
        }
    }

    public IPropertySelectionModel getConditionModel() {
        Map<String, String> types2Labels = new LinkedHashMap<String, String>();
        for (CONDITION value : CONDITION.values()) {
            types2Labels.put(value.toString(), getMessages().getMessage(value.toString()));
        }
        return new NamedValuesSelectionModel(types2Labels);
    }

    public IPropertySelectionModel getRelationModel() {
        String condition = getRecipeCondition().getCondition();
        Map<String, String> types2Labels = new LinkedHashMap<String, String>();
        types2Labels.put(RELATION.IS.toString(), getMessages().getMessage(RELATION.IS.toString()));

        if (condition.equals(CONDITION.CLIENT.toString()) || condition.equals(CONDITION.MEDIA_TYPE.toString())) {
            types2Labels.put(RELATION.NOT.toString(), getMessages().getMessage(RELATION.NOT.toString()));
        } else {
            if (!condition.equals(CONDITION.TICK_INTERVAL.toString())) {
                types2Labels.put(RELATION.GREATER.toString(), getMessages().getMessage(RELATION.GREATER.toString()));
                types2Labels.put(RELATION.LESS.toString(), getMessages().getMessage(RELATION.LESS.toString()));
            }
        }
        return new SafeNamedValuesSelectionModel(types2Labels);
    }

    public boolean isNumberComponent() {
        String condition = getRecipeCondition().getCondition();
        return !(condition.equals(CONDITION.CLIENT.toString()) || condition.equals(CONDITION.MEDIA_TYPE.toString()));
    }
}
