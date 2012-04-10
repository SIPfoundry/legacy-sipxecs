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

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
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
import org.sipfoundry.sipxconfig.openacd.OpenAcdClient;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition.CONDITION;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition.RELATION;
import org.sipfoundry.sipxconfig.openacd.RecipeStepBean;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdRecipeConditionComponent extends BaseComponent {

    private static final String HOUR_FORMAT = "HH";
    private static final String VOICE = "voice";
    private static final String VOICEMAIL = "voicemail";
    private static final String EMAIL = "email";
    private static final String CHAT = "chat";

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

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

    public String getHelpText() {
        return getMessages().getMessage("description." + getRecipeCondition().getCondition());
    }

    public OpenAcdClient getSelectedClient() {
        OpenAcdClient client = null;
        if (isClientComponent()) {
            client = getOpenAcdContext().getClientByIdentity(getRecipeCondition().getValueCondition());
        }
        return client;
    }

    public void setSelectedClient(OpenAcdClient client) {
        getRecipeCondition().setValueCondition(client.getIdentity());
    }

    public Integer getDayOfWeek() {
        Integer dayOfWeek = null;
        if (isDayOfWeekComponent()) {
            try {
                dayOfWeek = new Integer(getRecipeCondition().getValueCondition());
            } catch (NumberFormatException e) {
                // do not format value condition, keep null
                dayOfWeek = null;
            }
        }
        return dayOfWeek;
    }

    public void setDayOfWeek(Integer dayOfWeek) {
        getRecipeCondition().setValueCondition(dayOfWeek.toString());
    }

    public Date getHour() {
        Date hour = null;
        if (isHourComponent()) {
            DateFormat formatter = new SimpleDateFormat(HOUR_FORMAT);
            try {
                hour = formatter.parse(getRecipeCondition().getValueCondition());
            } catch (ParseException e) {
                // do not format value condition, keep null
                hour = null;
            }
        }
        return hour;
    }

    public void setHour(Date time) {
        if (time == null) {
            getRecipeCondition().setValueCondition(null);
            return;
        }
        DateFormat formatter = new SimpleDateFormat(HOUR_FORMAT);
        String s = formatter.format(time);
        getRecipeCondition().setValueCondition(s);
    }

    public String getMediaType() {
        String mediaType = null;
        if (isMediaTypeComponent()) {
            mediaType = getRecipeCondition().getValueCondition();
        }
        return mediaType;
    }

    public void setMediaType(String mediaType) {
        getRecipeCondition().setValueCondition(mediaType);
    }

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

        if (condition.equals(CONDITION.CLIENT.toString()) || condition.equals(CONDITION.MEDIA_TYPE.toString())
                || condition.equals(CONDITION.CALLER_NAME.toString())
                || condition.equals(CONDITION.CALLER_ID.toString())) {
            types2Labels.put(RELATION.NOT.toString(), getMessages().getMessage(RELATION.NOT.toString()));
        } else {
            if (!condition.equals(CONDITION.TICK_INTERVAL.toString())) {
                types2Labels.put(RELATION.GREATER.toString(), getMessages().getMessage(RELATION.GREATER.toString()));
                types2Labels.put(RELATION.LESS.toString(), getMessages().getMessage(RELATION.LESS.toString()));
            }
        }
        return new SafeNamedValuesSelectionModel(types2Labels);
    }

    public IPropertySelectionModel getSelectDayOfWeekModel() {
        Map<Integer, String> map = new HashMap<Integer, String>();
        map.put(Calendar.SUNDAY, getMessages().getMessage("sunday"));
        map.put(Calendar.MONDAY, getMessages().getMessage("monday"));
        map.put(Calendar.TUESDAY, getMessages().getMessage("tuesday"));
        map.put(Calendar.WEDNESDAY, getMessages().getMessage("wednesday"));
        map.put(Calendar.THURSDAY, getMessages().getMessage("thursday"));
        map.put(Calendar.FRIDAY, getMessages().getMessage("friday"));
        map.put(Calendar.SATURDAY, getMessages().getMessage("saturday"));
        return new NamedValuesSelectionModel(map);
    }

    public boolean isNumberComponent() {
        String condition = getRecipeCondition().getCondition();
        return (condition.equals(CONDITION.TICK_INTERVAL.toString())
                || condition.equals(CONDITION.AGENTS_AVAILABLE.toString())
                || condition.equals(CONDITION.AGENTS_ELIGIBLE.toString())
                || condition.equals(CONDITION.CALLS_IN_QUEUE.toString())
                || condition.equals(CONDITION.POSITION_IN_QUEUE.toString()) || condition
                .equals(CONDITION.CLIENTS_QUEUED.toString()));
    }

    public IPropertySelectionModel getSelectMediaTypeModel() {
        Map<String, String> map = new HashMap<String, String>();
        map.put(VOICE, getMessages().getMessage(VOICE));
        map.put(VOICEMAIL, getMessages().getMessage(VOICEMAIL));
        map.put(EMAIL, getMessages().getMessage(EMAIL));
        map.put(CHAT, getMessages().getMessage(CHAT));
        return new NamedValuesSelectionModel(map);
    }

    public boolean isClientComponent() {
        String condition = getRecipeCondition().getCondition();
        return condition.equals(CONDITION.CLIENT.toString());
    }

    public boolean isDayOfWeekComponent() {
        String condition = getRecipeCondition().getCondition();
        return condition.equals(CONDITION.DAY_OF_WEEK.toString());
    }

    public boolean isHourComponent() {
        String condition = getRecipeCondition().getCondition();
        return condition.equals(CONDITION.HOUR.toString());
    }

    public boolean isMediaTypeComponent() {
        String condition = getRecipeCondition().getCondition();
        return condition.equals(CONDITION.MEDIA_TYPE.toString());
    }
}
