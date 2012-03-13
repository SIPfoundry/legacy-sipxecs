/*
 *
 *  Copyright (C) 2012 PATLive, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdAgentGroupsResource.java - A Restlet to read group data from OpenACD within SipXecs
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
import static org.sipfoundry.sipxconfig.rest.RestUtilities.REQUEST_ATTRIBUTE_ID;
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
import org.sipfoundry.sipxconfig.openacd.OpenAcdClient;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueue;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkill;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdAgentGroupRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdClientRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdQueueRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdSkillRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class OpenAcdAgentGroupsResource extends Resource {

    private static final String ELEMENT_NAME_AGENTGROUPSBUNDLE = "openacd-agent-group";
    private static final String ELEMENT_NAME_GROUP = "group";
    private static final String ELEMENT_NAME_SKILL = "skill";
    private static final String ELEMENT_NAME_QUEUE = "queue";
    private static final String ELEMENT_NAME_CLIENT = "client";

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
        OpenAcdAgentGroup agentGroup;
        OpenAcdAgentGroupRestInfoFull agentGroupRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                agentGroup = m_openAcdContext.getAgentGroupById(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                agentGroupRestInfo = createAgentGroupRestInfo(agentGroup);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new OpenAcdAgentGroupRepresentation(variant.getMediaType(), agentGroupRestInfo);
        }

        // if not single, process request for list
        List<OpenAcdAgentGroup> agentGroups = m_openAcdContext.getAgentGroups();
        List<OpenAcdAgentGroupRestInfoFull> agentGroupsRestInfo = new ArrayList<OpenAcdAgentGroupRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort if specified
        sortGroups(agentGroups);

        // set requested based on pagination and get resulting metadata
        metadataRestInfo = addAgentGroups(agentGroupsRestInfo, agentGroups);

        // create final restinfo
        OpenAcdAgentGroupsBundleRestInfo agentGroupsBundleRestInfo =
                new OpenAcdAgentGroupsBundleRestInfo(agentGroupsRestInfo, metadataRestInfo);

        return new OpenAcdAgentGroupsRepresentation(variant.getMediaType(), agentGroupsBundleRestInfo);
    }

    // PUT - Update or Add single Group
    // --------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get item from request body
        OpenAcdAgentGroupRepresentation representation = new OpenAcdAgentGroupRepresentation(entity);
        OpenAcdAgentGroupRestInfoFull agentGroupRestInfo = representation.getObject();
        OpenAcdAgentGroup agentGroup;

        // validate input for update or create
        ValidationInfo validationInfo = validate(agentGroupRestInfo);

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
                agentGroup = m_openAcdContext.getAgentGroupById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateAgentGroup(agentGroup, agentGroupRestInfo);
                m_openAcdContext.saveAgentGroup(agentGroup);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, agentGroup.getId());
            return;
        }

        // if not single, add new item
        try {
            agentGroup = createAgentGroup(agentGroupRestInfo);
            m_openAcdContext.saveAgentGroup(agentGroup);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, agentGroup.getId());
    }

    // DELETE - Delete single Group
    // ----------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdAgentGroup agentGroup;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                agentGroup = m_openAcdContext.getAgentGroupById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            try {
                m_openAcdContext.deleteAgentGroup(agentGroup);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, agentGroup.getId());
            return;
        }

        // no id string
        RestUtilities.setResponse(getResponse(), ERROR_MISSING_ID);
    }

    // Helper functions
    // ----------------

    // basic interface level validation of data provided through REST interface for creation or
    // update
    // may also contain clean up of input data
    // may create another validation function if different rules needed for update v. create
    private ValidationInfo validate(OpenAcdAgentGroupRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String nameObject = "Name";

        String name = restInfo.getName();

        validationInfo.checkString(name, nameObject, StringConstraint.NOT_EMPTY);
        validationInfo.checkString(name, nameObject, StringConstraint.ONLY_LETTER_NUMBER_DASH_UNDERSCORE);

        return validationInfo;
    }

    private OpenAcdAgentGroupRestInfoFull createAgentGroupRestInfo(OpenAcdAgentGroup agentGroup) {
        OpenAcdAgentGroupRestInfoFull agentGroupRestInfo;
        List<OpenAcdSkillRestInfo> skillsRestInfo;
        List<OpenAcdQueueRestInfo> queuesRestInfo;
        List<OpenAcdClientRestInfo> clientsRestInfo;

        skillsRestInfo = createSkillsRestInfo(agentGroup);
        queuesRestInfo = createQueuesRestInfo(agentGroup);
        clientsRestInfo = createClientsRestInfo(agentGroup);
        agentGroupRestInfo =
                new OpenAcdAgentGroupRestInfoFull(agentGroup, skillsRestInfo, queuesRestInfo, clientsRestInfo);

        return agentGroupRestInfo;
    }

    private MetadataRestInfo addAgentGroups(List<OpenAcdAgentGroupRestInfoFull> agentGroupsRestInfo,
            List<OpenAcdAgentGroup> agentGroups) {
        OpenAcdAgentGroup agentGroup;
        OpenAcdAgentGroupRestInfoFull agentGroupRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, agentGroups.size());

        // create list of group restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            agentGroup = agentGroups.get(index);
            agentGroupRestInfo = createAgentGroupRestInfo(agentGroup);
            agentGroupsRestInfo.add(agentGroupRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    private List<OpenAcdSkillRestInfo> createSkillsRestInfo(OpenAcdAgentGroup agentGroup) {
        List<OpenAcdSkillRestInfo> skillsRestInfo;
        OpenAcdSkillRestInfo skillRestInfo;

        // create list of skill restinfos for single group
        Set<OpenAcdSkill> groupSkills = agentGroup.getSkills();
        skillsRestInfo = new ArrayList<OpenAcdSkillRestInfo>(groupSkills.size());

        for (OpenAcdSkill groupSkill : groupSkills) {
            skillRestInfo = new OpenAcdSkillRestInfo(groupSkill);
            skillsRestInfo.add(skillRestInfo);
        }

        return skillsRestInfo;
    }

    private List<OpenAcdQueueRestInfo> createQueuesRestInfo(OpenAcdAgentGroup agentGroup) {
        List<OpenAcdQueueRestInfo> queuesRestInfo;
        OpenAcdQueueRestInfo queueRestInfo;

        // create list of queue restinfos for single group
        Set<OpenAcdQueue> groupQueues = agentGroup.getQueues();
        queuesRestInfo = new ArrayList<OpenAcdQueueRestInfo>(groupQueues.size());

        for (OpenAcdQueue groupQueue : groupQueues) {
            queueRestInfo = new OpenAcdQueueRestInfo(groupQueue);
            queuesRestInfo.add(queueRestInfo);
        }

        return queuesRestInfo;
    }

    private List<OpenAcdClientRestInfo> createClientsRestInfo(OpenAcdAgentGroup agentGroup) {
        List<OpenAcdClientRestInfo> clientsRestInfo;
        OpenAcdClientRestInfo clientRestInfo;

        // create list of client restinfos for single group
        Set<OpenAcdClient> groupClients = agentGroup.getClients();
        clientsRestInfo = new ArrayList<OpenAcdClientRestInfo>(groupClients.size());

        for (OpenAcdClient groupClient : groupClients) {
            clientRestInfo = new OpenAcdClientRestInfo(groupClient);
            clientsRestInfo.add(clientRestInfo);
        }

        return clientsRestInfo;
    }

    @SuppressWarnings("unchecked")
    private void sortGroups(List<OpenAcdAgentGroup> agentGroups) {
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
                Collections.sort(agentGroups, new NameComparator());
            } else {
                Collections.sort(agentGroups, new ReverseComparator(new NameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(agentGroups, new DescriptionComparator());
            } else {
                Collections.sort(agentGroups, new ReverseComparator(new DescriptionComparator()));
            }

            break;

        default:
            break;
        }
    }

    private void updateAgentGroup(OpenAcdAgentGroup agentGroup, OpenAcdAgentGroupRestInfoFull agentGroupRestInfo) {
        agentGroup.setName(agentGroupRestInfo.getName());
        agentGroup.setDescription(agentGroupRestInfo.getDescription());

        addLists(agentGroup, agentGroupRestInfo);
    }

    private OpenAcdAgentGroup createAgentGroup(OpenAcdAgentGroupRestInfoFull agentGroupRestInfo) {
        OpenAcdAgentGroup agentGroup = new OpenAcdAgentGroup();

        updateAgentGroup(agentGroup, agentGroupRestInfo);

        return agentGroup;
    }

    private void addLists(OpenAcdAgentGroup agentGroup, OpenAcdAgentGroupRestInfoFull agentGroupRestInfo) {
        // remove all current skills
        agentGroup.getSkills().clear();

        // set skills
        OpenAcdSkill skill;
        List<OpenAcdSkillRestInfo> skillsRestInfo = agentGroupRestInfo.getSkills();
        for (OpenAcdSkillRestInfo skillRestInfo : skillsRestInfo) {
            skill = m_openAcdContext.getSkillById(skillRestInfo.getId());
            agentGroup.addSkill(skill);
        }

        // remove all current queues
        agentGroup.getQueues().clear();

        // set queues
        OpenAcdQueue queue;
        List<OpenAcdQueueRestInfo> queuesRestInfo = agentGroupRestInfo.getQueues();
        for (OpenAcdQueueRestInfo queueRestInfo : queuesRestInfo) {
            queue = m_openAcdContext.getQueueById(queueRestInfo.getId());
            agentGroup.addQueue(queue);
        }

        // remove all current clients
        agentGroup.getClients().clear();

        // set clients
        OpenAcdClient client;
        List<OpenAcdClientRestInfo> clientsRestInfo = agentGroupRestInfo.getClients();
        for (OpenAcdClientRestInfo clientRestInfo : clientsRestInfo) {
            client = m_openAcdContext.getClientById(clientRestInfo.getId());
            agentGroup.addClient(client);
        }
    }

    // Sorting comparator classes
    // --------------------------

    private static class NameComparator implements Comparator<OpenAcdAgentGroup> {
        @Override
        public int compare(OpenAcdAgentGroup item1, OpenAcdAgentGroup item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class DescriptionComparator implements Comparator<OpenAcdAgentGroup> {
        @Override
        public int compare(OpenAcdAgentGroup item1, OpenAcdAgentGroup item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    // REST Representations
    // --------------------

    static class OpenAcdAgentGroupsRepresentation extends XStreamRepresentation<OpenAcdAgentGroupsBundleRestInfo> {

        public OpenAcdAgentGroupsRepresentation(MediaType mediaType, OpenAcdAgentGroupsBundleRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdAgentGroupsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_AGENTGROUPSBUNDLE, OpenAcdAgentGroupsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_GROUP, OpenAcdAgentGroupRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_SKILL, OpenAcdSkillRestInfo.class);
            xstream.alias(ELEMENT_NAME_QUEUE, OpenAcdQueueRestInfo.class);
            xstream.alias(ELEMENT_NAME_CLIENT, OpenAcdClientRestInfo.class);
        }
    }

    static class OpenAcdAgentGroupRepresentation extends XStreamRepresentation<OpenAcdAgentGroupRestInfoFull> {

        public OpenAcdAgentGroupRepresentation(MediaType mediaType, OpenAcdAgentGroupRestInfoFull object) {
            super(mediaType, object);
        }

        public OpenAcdAgentGroupRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_GROUP, OpenAcdAgentGroupRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_SKILL, OpenAcdSkillRestInfo.class);
            xstream.alias(ELEMENT_NAME_QUEUE, OpenAcdQueueRestInfo.class);
            xstream.alias(ELEMENT_NAME_CLIENT, OpenAcdClientRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdAgentGroupsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<OpenAcdAgentGroupRestInfoFull> m_groups;

        public OpenAcdAgentGroupsBundleRestInfo(List<OpenAcdAgentGroupRestInfoFull> agentGroups,
                MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_groups = agentGroups;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<OpenAcdAgentGroupRestInfoFull> getGroups() {
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
