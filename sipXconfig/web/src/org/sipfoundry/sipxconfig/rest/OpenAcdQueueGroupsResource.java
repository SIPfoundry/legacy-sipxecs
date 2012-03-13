/*
 *
 *  Copyright (C) 2012 PATLive, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdQueueGroupsResource.java - A Restlet to read group data from OpenACD within SipXecs
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.

 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

package org.sipfoundry.sipxconfig.rest;

import static org.restlet.data.MediaType.APPLICATION_JSON;
import static org.restlet.data.MediaType.TEXT_XML;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.COLON;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.REQUEST_ATTRIBUTE_ID;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.ACTION_ANNOUNCE;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.ACTION_SET_PRIORITY;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.CONDITION_CALLER_ID;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.CONDITION_CALLER_NAME;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.CONDITION_CLIENT;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.CONDITION_EQUALITY;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.CONDITION_IS;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.CONDITION_MEDIA_TYPE;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.CONDITION_WEEKDAY;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.EQUALITY_RELATION;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.IS_RELATION;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.MEDIA_VALUES;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.VALUE_ACTION_VALUE;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.VALUE_CONDITION;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.VALUE_RELATION;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ConditionInfo.VALUE_VALUE_CONDITION;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_CREATE_FAILED;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_DELETE_FAILED;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_ID_INVALID;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_MISSING_ID;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_OBJECT_NOT_FOUND;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_READ_FAILED;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_UPDATE_FAILED;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.SUCCESS_CREATED;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.SUCCESS_DELETED;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.SUCCESS_UPDATED;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.contrib.table.model.common.ReverseComparator;
import org.restlet.Context;
import org.restlet.data.Form;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueueGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeAction;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeCondition;
import org.sipfoundry.sipxconfig.openacd.OpenAcdRecipeStep;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkill;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdAgentGroupRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdQueueGroupRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdRecipeActionRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdRecipeConditionRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdRecipeStepRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdSkillRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class OpenAcdQueueGroupsResource extends Resource {

    private static final String ELEMENT_NAME_QUEUEGROUPBUNDLE = "openacd-queue-group";
    private static final String ELEMENT_NAME_QUEUEGROUP = "group";
    private static final String ELEMENT_NAME_SKILL = "skill";
    private static final String ELEMENT_NAME_AGENTGROUP = "agentGroup";
    private static final String ELEMENT_NAME_STEP = "step";
    private static final String ELEMENT_NAME_CONDITION = "condition";
    private static final String ELEMENT_NAME_ACTION = "action";

    private OpenAcdContext m_openAcdContext;
    private Form m_form;

    // use to define all possible sort fields
    private enum SortField {
        NAME, DESCRIPTION, NONE;

        public static SortField toSortField(String fieldString) {
            if (fieldString == null) {
                return NONE;
            }

            try {
                return valueOf(fieldString.toUpperCase());
            } catch (Exception ex) {
                return NONE;
            }
        }
    }

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_XML));
        getVariants().add(new Variant(APPLICATION_JSON));

        // pull parameters from url
        m_form = getRequest().getResourceRef().getQueryAsForm();
    }

    // GET - Retrieve Groups and single Group
    // --------------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdQueueGroup queueGroup;
        OpenAcdQueueGroupRestInfoFull queueGroupRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                queueGroup = m_openAcdContext.getQueueGroupById(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                queueGroupRestInfo = createQueueGroupRestInfo(queueGroup);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new OpenAcdQueueGroupRepresentation(variant.getMediaType(), queueGroupRestInfo);
        }

        // if not single, process request for list
        List<OpenAcdQueueGroup> queueGroups = m_openAcdContext.getQueueGroups();
        List<OpenAcdQueueGroupRestInfoFull> queueGroupsRestInfo = new ArrayList<OpenAcdQueueGroupRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort if specified
        sortQueueGroups(queueGroups);

        // set requested based on pagination and get resulting metadata
        metadataRestInfo = addQueueGroups(queueGroupsRestInfo, queueGroups);

        // create final restinfo
        OpenAcdQueueGroupsBundleRestInfo queueGroupsBundleRestInfo =
                new OpenAcdQueueGroupsBundleRestInfo(queueGroupsRestInfo, metadataRestInfo);

        return new OpenAcdQueueGroupsRepresentation(variant.getMediaType(), queueGroupsBundleRestInfo);
    }

    // PUT - Update or Add single Group
    // --------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get item from request body
        OpenAcdQueueGroupRepresentation representation = new OpenAcdQueueGroupRepresentation(entity);
        OpenAcdQueueGroupRestInfoFull queueGroupRestInfo = representation.getObject();
        OpenAcdQueueGroup queueGroup;

        // validate input for update or create
        ValidationInfo validationInfo = validate(queueGroupRestInfo);

        if (!validationInfo.getValid()) {
            RestUtilities.setResponseError(getResponse(), validationInfo.getResponseCode(), validationInfo
                    .getMessage());
            return;
        }

        // if have id then update single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                queueGroup = m_openAcdContext.getQueueGroupById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateQueueGroup(queueGroup, queueGroupRestInfo);
                m_openAcdContext.saveQueueGroup(queueGroup);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, queueGroup.getId());
            return;
        }

        // if not single, add new item
        try {
            queueGroup = createQueueGroup(queueGroupRestInfo);
            m_openAcdContext.saveQueueGroup(queueGroup);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, queueGroup.getId());
    }

    // DELETE - Delete single Group
    // --------------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdQueueGroup queueGroup;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                queueGroup = m_openAcdContext.getQueueGroupById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            try {
                m_openAcdContext.deleteQueueGroup(queueGroup);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, queueGroup.getId());
            return;
        }

        // no id string
        RestUtilities.setResponse(getResponse(), ERROR_MISSING_ID);
    }

    // Helper functions
    // ----------------

    // basic interface level validation of data provided through REST interface for creation or
    // update may also contain clean up of input data
    // may create another validation function if different rules needed for update and create
    private ValidationInfo validate(OpenAcdQueueGroupRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String nameObject = "Name";

        String name = restInfo.getName();

        validationInfo.checkString(name, nameObject, StringConstraint.NOT_EMPTY);
        validationInfo.checkString(name, nameObject, StringConstraint.ONLY_LETTER_NUMBER_DASH_UNDERSCORE);

        // check all steps
        for (int i = 0; i < restInfo.getSteps().size(); i++) {
            OpenAcdRecipeActionRestInfo action = restInfo.getSteps().get(i).getAction();

            validationInfo.checkString(action.getAction(), "Action", StringConstraint.NOT_EMPTY);

            if (action.getAction().equals(ACTION_ANNOUNCE) || action.getAction().equals(ACTION_SET_PRIORITY)) {
                validationInfo.checkString(action.getActionValue(),
                        action.getAction() + COLON + VALUE_ACTION_VALUE, StringConstraint.NOT_EMPTY);
            }

            if (action.getAction().equals(ACTION_SET_PRIORITY)) {
                validationInfo.checkString(action.getActionValue(),
                        action.getAction() + COLON + VALUE_ACTION_VALUE, StringConstraint.ONLY_NUMBER_ASTERISK);
            }

            // for this step check all conditions
            for (int k = 0; k < restInfo.getSteps().get(i).getConditions().size(); k++) {
                OpenAcdRecipeConditionRestInfo condition = restInfo.getSteps().get(i).getConditions().get(k);

                validationInfo.checkString(condition.getCondition(), VALUE_CONDITION, StringConstraint.NOT_EMPTY);
                validationInfo.checkString(condition.getRelation(), VALUE_RELATION, StringConstraint.NOT_EMPTY);
                validationInfo.checkString(condition.getValueCondition(), VALUE_VALUE_CONDITION,
                        StringConstraint.NOT_EMPTY);

                if (CONDITION_IS.contains(condition.getCondition())) {
                    validationInfo.checkMember(condition.getRelation(), condition.getCondition() + COLON
                        + VALUE_RELATION, IS_RELATION);
                }

                if (CONDITION_EQUALITY.contains(condition.getCondition())) {
                    validationInfo.checkMember(condition.getRelation(), condition.getCondition() + COLON
                        + VALUE_RELATION, EQUALITY_RELATION);
                }

                if (condition.getCondition().equals(CONDITION_MEDIA_TYPE)) {
                    validationInfo.checkMember(condition.getValueCondition(), condition.getCondition() + COLON
                        + VALUE_VALUE_CONDITION, MEDIA_VALUES);
                }

                if (condition.getCondition().equals(CONDITION_WEEKDAY)) {
                    validationInfo.checkStringRange(condition.getValueCondition(), condition.getCondition() + COLON
                        + VALUE_VALUE_CONDITION, 1, 7);
                }

                if (!(condition.getCondition().equals(CONDITION_CALLER_ID)
                    || condition.getCondition().equals(CONDITION_CALLER_NAME)
                    || condition.getCondition().equals(CONDITION_MEDIA_TYPE) || condition.getCondition().equals(
                        CONDITION_CLIENT))) {
                    validationInfo.checkString(condition.getValueCondition(), condition.getCondition() + COLON
                        + VALUE_VALUE_CONDITION, StringConstraint.ONLY_NUMBER_ASTERISK);
                }
            }
        }

        return validationInfo;
    }

    private OpenAcdQueueGroupRestInfoFull createQueueGroupRestInfo(OpenAcdQueueGroup queueGroup) {
        OpenAcdQueueGroupRestInfoFull queueGroupRestInfo;
        List<OpenAcdSkillRestInfo> skillsRestInfo;
        List<OpenAcdAgentGroupRestInfo> agentGroupRestInfo;
        List<OpenAcdRecipeStepRestInfo> recipeStepRestInfo;

        skillsRestInfo = createSkillsRestInfo(queueGroup.getSkills());
        agentGroupRestInfo = createAgentGroupsRestInfo(queueGroup);
        recipeStepRestInfo = createRecipeStepsRestInfo(queueGroup);
        queueGroupRestInfo =
                new OpenAcdQueueGroupRestInfoFull(queueGroup, skillsRestInfo, agentGroupRestInfo,
                        recipeStepRestInfo);

        return queueGroupRestInfo;
    }

    private List<OpenAcdSkillRestInfo> createSkillsRestInfo(Set<OpenAcdSkill> groupSkills) {
        List<OpenAcdSkillRestInfo> skillsRestInfo;
        OpenAcdSkillRestInfo skillRestInfo;

        // create list of skill restinfos for single group
        skillsRestInfo = new ArrayList<OpenAcdSkillRestInfo>(groupSkills.size());

        for (OpenAcdSkill groupSkill : groupSkills) {
            skillRestInfo = new OpenAcdSkillRestInfo(groupSkill);
            skillsRestInfo.add(skillRestInfo);
        }

        return skillsRestInfo;
    }

    private List<OpenAcdAgentGroupRestInfo> createAgentGroupsRestInfo(OpenAcdQueueGroup queueGroup) {
        List<OpenAcdAgentGroupRestInfo> agentGroupsRestInfo;
        OpenAcdAgentGroupRestInfo agentGroupRestInfo;

        // create list of agent group restinfos for single group
        Set<OpenAcdAgentGroup> groupAgentGroups = queueGroup.getAgentGroups();
        agentGroupsRestInfo = new ArrayList<OpenAcdAgentGroupRestInfo>(groupAgentGroups.size());

        for (OpenAcdAgentGroup groupAgentGroup : groupAgentGroups) {
            agentGroupRestInfo = new OpenAcdAgentGroupRestInfo(groupAgentGroup);
            agentGroupsRestInfo.add(agentGroupRestInfo);
        }

        return agentGroupsRestInfo;
    }

    private List<OpenAcdRecipeStepRestInfo> createRecipeStepsRestInfo(OpenAcdQueueGroup queueGroup) {
        List<OpenAcdRecipeStepRestInfo> recipeStepsRestInfo;
        OpenAcdRecipeStepRestInfo recipeStepRestInfo;

        Set<OpenAcdRecipeStep> groupRecipeSteps = queueGroup.getSteps();
        recipeStepsRestInfo = new ArrayList<OpenAcdRecipeStepRestInfo>(groupRecipeSteps.size());

        for (OpenAcdRecipeStep groupRecipeStep : groupRecipeSteps) {
            recipeStepRestInfo =
                    new OpenAcdRecipeStepRestInfo(groupRecipeStep, createRecipeActionRestInfo(groupRecipeStep),
                            createRecipeConditionsRestInfo(groupRecipeStep));
            recipeStepsRestInfo.add(recipeStepRestInfo);
        }
        return recipeStepsRestInfo;
    }

    private OpenAcdRecipeActionRestInfo createRecipeActionRestInfo(OpenAcdRecipeStep step) {
        OpenAcdRecipeActionRestInfo recipeActionRestInfo;
        List<OpenAcdSkillRestInfo> skillsRestInfo;

        // get skills associated with action
        skillsRestInfo = createSkillsRestInfo(step.getAction().getSkills());
        recipeActionRestInfo = new OpenAcdRecipeActionRestInfo(step.getAction(), skillsRestInfo);

        return recipeActionRestInfo;
    }

    private List<OpenAcdRecipeConditionRestInfo> createRecipeConditionsRestInfo(OpenAcdRecipeStep step) {
        List<OpenAcdRecipeConditionRestInfo> recipeConditionsRestInfo;
        OpenAcdRecipeConditionRestInfo recipeConditionRestInfo;

        List<OpenAcdRecipeCondition> groupRecipeConditions = step.getConditions();
        recipeConditionsRestInfo = new ArrayList<OpenAcdRecipeConditionRestInfo>(groupRecipeConditions.size());

        for (OpenAcdRecipeCondition groupRecipeCondition : groupRecipeConditions) {
            recipeConditionRestInfo = new OpenAcdRecipeConditionRestInfo(groupRecipeCondition);
            recipeConditionsRestInfo.add(recipeConditionRestInfo);
        }

        return recipeConditionsRestInfo;
    }

    private MetadataRestInfo addQueueGroups(List<OpenAcdQueueGroupRestInfoFull> queueGroupsRestInfo,
            List<OpenAcdQueueGroup> queueGroups) {
        OpenAcdQueueGroup queueGroup;
        OpenAcdQueueGroupRestInfoFull queueGroupRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, queueGroups.size());

        // create list of group restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            queueGroup = queueGroups.get(index);
            queueGroupRestInfo = createQueueGroupRestInfo(queueGroup);
            queueGroupsRestInfo.add(queueGroupRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortQueueGroups(List<OpenAcdQueueGroup> queueGroups) {
        // sort groups if requested
        SortInfo sortInfo = RestUtilities.calculateSorting(m_form);

        if (!sortInfo.getSort()) {
            return;
        }

        SortField sortField = SortField.toSortField(sortInfo.getSortField());
        boolean sortForward = sortInfo.getDirectionForward();

        switch (sortField) {
        case NAME:
            if (sortForward) {
                Collections.sort(queueGroups, new NameComparator());
            } else {
                Collections.sort(queueGroups, new ReverseComparator(new NameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(queueGroups, new DescriptionComparator());
            } else {
                Collections.sort(queueGroups, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateQueueGroup(OpenAcdQueueGroup queueGroup, OpenAcdQueueGroupRestInfoFull queueGroupRestInfo) {
        queueGroup.setName(queueGroupRestInfo.getName());
        queueGroup.setDescription(queueGroupRestInfo.getDescription());

        addLists(queueGroup, queueGroupRestInfo);
    }

    private OpenAcdQueueGroup createQueueGroup(OpenAcdQueueGroupRestInfoFull queueGroupRestInfo) {
        OpenAcdQueueGroup queueGroup = new OpenAcdQueueGroup();

        updateQueueGroup(queueGroup, queueGroupRestInfo);

        return queueGroup;
    }

    private void addLists(OpenAcdQueueGroup queueGroup, OpenAcdQueueGroupRestInfoFull queueGroupRestInfo) {
        // remove all skills
        queueGroup.getSkills().clear();

        // set skills
        OpenAcdSkill skill;
        List<OpenAcdSkillRestInfo> skillsRestInfo = queueGroupRestInfo.getSkills();
        for (OpenAcdSkillRestInfo skillRestInfo : skillsRestInfo) {
            skill = m_openAcdContext.getSkillById(skillRestInfo.getId());
            queueGroup.addSkill(skill);
        }

        // remove all agent groups
        queueGroup.getAgentGroups().clear();

        // set agent groups
        OpenAcdAgentGroup agentGroup;
        List<OpenAcdAgentGroupRestInfo> agentGroupsRestInfo = queueGroupRestInfo.getAgentGroups();
        for (OpenAcdAgentGroupRestInfo agentGroupRestInfo : agentGroupsRestInfo) {
            agentGroup = m_openAcdContext.getAgentGroupById(agentGroupRestInfo.getId());
            queueGroup.addAgentGroup(agentGroup);
        }

        // remove all current steps
        queueGroup.getSteps().clear();

        // set steps
        OpenAcdRecipeStep step;
        OpenAcdRecipeCondition condition;
        OpenAcdRecipeAction action;
        OpenAcdRecipeActionRestInfo actionRestInfo;

        List<OpenAcdRecipeStepRestInfo> recipeStepsRestInfo = queueGroupRestInfo.getSteps();
        for (OpenAcdRecipeStepRestInfo recipeStepRestInfo : recipeStepsRestInfo) {
            step = new OpenAcdRecipeStep();
            step.setFrequency(recipeStepRestInfo.getFrequency());

            // add conditions
            step.getConditions().clear();
            for (OpenAcdRecipeConditionRestInfo recipeConditionRestInfo : recipeStepRestInfo.getConditions()) {
                condition = new OpenAcdRecipeCondition();
                condition.setCondition(recipeConditionRestInfo.getCondition());
                condition.setRelation(recipeConditionRestInfo.getRelation());
                condition.setValueCondition(recipeConditionRestInfo.getValueCondition());

                step.addCondition(condition);
            }

            // add action
            action = new OpenAcdRecipeAction();
            actionRestInfo = recipeStepRestInfo.getAction();
            action.setAction(actionRestInfo.getAction());
            action.setActionValue(actionRestInfo.getActionValue());

            // set action skills (separate from skills assigned to queue
            for (OpenAcdSkillRestInfo skillRestInfo : actionRestInfo.getSkills()) {
                skill = m_openAcdContext.getSkillById(skillRestInfo.getId());
                action.addSkill(skill);
            }

            step.setAction(action);

            queueGroup.addStep(step);
        }
    }

    // Sorting comparator classes
    // --------------------------

    private static class NameComparator implements Comparator<OpenAcdQueueGroup> {
        @Override
        public int compare(OpenAcdQueueGroup item1, OpenAcdQueueGroup item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class DescriptionComparator implements Comparator<OpenAcdQueueGroup> {
        @Override
        public int compare(OpenAcdQueueGroup item1, OpenAcdQueueGroup item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    // REST Representations
    // --------------------

    static class OpenAcdQueueGroupsRepresentation extends XStreamRepresentation<OpenAcdQueueGroupsBundleRestInfo> {

        public OpenAcdQueueGroupsRepresentation(MediaType mediaType, OpenAcdQueueGroupsBundleRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdQueueGroupsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_QUEUEGROUPBUNDLE, OpenAcdQueueGroupsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_QUEUEGROUP, OpenAcdQueueGroupRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_SKILL, OpenAcdSkillRestInfo.class);
            xstream.alias(ELEMENT_NAME_AGENTGROUP, OpenAcdAgentGroupRestInfo.class);
            xstream.alias(ELEMENT_NAME_STEP, OpenAcdRecipeStepRestInfo.class);
            xstream.alias(ELEMENT_NAME_CONDITION, OpenAcdRecipeConditionRestInfo.class);
            xstream.alias(ELEMENT_NAME_ACTION, OpenAcdRecipeActionRestInfo.class);
        }
    }

    static class OpenAcdQueueGroupRepresentation extends XStreamRepresentation<OpenAcdQueueGroupRestInfoFull> {

        public OpenAcdQueueGroupRepresentation(MediaType mediaType, OpenAcdQueueGroupRestInfoFull object) {
            super(mediaType, object);
        }

        public OpenAcdQueueGroupRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_QUEUEGROUP, OpenAcdQueueGroupRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_SKILL, OpenAcdSkillRestInfo.class);
            xstream.alias(ELEMENT_NAME_AGENTGROUP, OpenAcdAgentGroupRestInfo.class);
            xstream.alias(ELEMENT_NAME_STEP, OpenAcdRecipeStepRestInfo.class);
            xstream.alias(ELEMENT_NAME_CONDITION, OpenAcdRecipeConditionRestInfo.class);
            xstream.alias(ELEMENT_NAME_ACTION, OpenAcdRecipeActionRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdQueueGroupsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<OpenAcdQueueGroupRestInfoFull> m_groups;

        public OpenAcdQueueGroupsBundleRestInfo(List<OpenAcdQueueGroupRestInfoFull> queueGroups,
                MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_groups = queueGroups;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<OpenAcdQueueGroupRestInfoFull> getGroups() {
            return m_groups;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

}
