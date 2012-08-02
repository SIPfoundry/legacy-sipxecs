/*
 *
 *  Copyright (C) 2012 PATLive, D. Waseem, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdAgentGroupsResource.java - A Restlet to read Agent data from OpenACD within SipXecs
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
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.contrib.table.model.common.ReverseComparator;
import org.restlet.Context;
import org.restlet.data.Form;
import org.restlet.data.MediaType;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgent;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdClient;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueue;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkill;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdAgentGroupRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdAgentRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdClientRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdQueueRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdSkillRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class OpenAcdAgentsResource extends Resource {

    private static final String ELEMENT_NAME_AGENTBUNDLE = "openacd-agent";
    private static final String ELEMENT_NAME_AGENT = "agent";
    private static final String ELEMENT_NAME_GROUP = "group";
    private static final String ELEMENT_NAME_SKILL = "skill";
    private static final String ELEMENT_NAME_QUEUE = "queue";
    private static final String ELEMENT_NAME_CLIENT = "client";

    private CoreContext m_coreContext;
    private OpenAcdContext m_openAcdContext;
    private Form m_form;

    // use to define all possible sort fields
    private enum SortField {
        NAME, GROUP, SECURITY, USERNAME, FIRSTNAME, LASTNAME, BRANCH, EXTENSION, NONE;

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

    // GET - Retrieve all and single Agent
    // -----------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdAgent agent;
        OpenAcdAgentRestInfoFull agentRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                agent = m_openAcdContext.getAgentById(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                agentRestInfo = createAgentRestInfo(agent);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new OpenAcdAgentRepresentation(variant.getMediaType(), agentRestInfo);
        }

        // if not single, process request for list
        List<OpenAcdAgent> agents = m_openAcdContext.getAgents();
        List<OpenAcdAgentRestInfoFull> agentsRestInfo = new ArrayList<OpenAcdAgentRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort if specified
        sortAgents(agents);

        // set requested agents groups and get resulting metadata
        metadataRestInfo = addAgents(agentsRestInfo, agents);

        // create final restinfo
        OpenAcdAgentsBundleRestInfo agentsBundleRestInfo =
                new OpenAcdAgentsBundleRestInfo(agentsRestInfo, metadataRestInfo);

        return new OpenAcdAgentsRepresentation(variant.getMediaType(), agentsBundleRestInfo);
    }

    // PUT - Update or Add single Agent
    // --------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get item from request body
        OpenAcdAgentRepresentation representation = new OpenAcdAgentRepresentation(entity);
        OpenAcdAgentRestInfoFull agentRestInfo = (OpenAcdAgentRestInfoFull) representation.getObject();
        OpenAcdAgent agent;

        // validate input for update or create
        ValidationInfo validationInfo = validate(agentRestInfo);

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
                agent = m_openAcdContext.getAgentById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateAgent(agent, agentRestInfo);
                m_openAcdContext.saveAgent(agent);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, agent.getId());
            return;
        }

        // if not single, add new item
        try {
            agent = createOpenAcdAgent(agentRestInfo);
            m_openAcdContext.saveAgent(agent);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, agent.getId());
    }

    // DELETE - Delete single Agent
    // ----------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdAgent agent;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                agent = m_openAcdContext.getAgentById(parameterInfo.getValue());
            } catch (Exception ex) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            try {
                m_openAcdContext.deleteAgent(agent);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, agent.getId());
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
    private ValidationInfo validate(OpenAcdAgentRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        return validationInfo;
    }

    private OpenAcdAgentRestInfoFull createAgentRestInfo(OpenAcdAgent agent) {
        OpenAcdAgentRestInfoFull agentRestInfo;
        List<OpenAcdSkillRestInfo> skillsRestInfo;
        List<OpenAcdQueueRestInfo> queuesRestInfo;
        List<OpenAcdClientRestInfo> clientsRestInfo;

        skillsRestInfo = createSkillsRestInfo(agent);
        queuesRestInfo = createQueuesRestInfo(agent);
        clientsRestInfo = createClientRestInfo(agent);
        agentRestInfo = new OpenAcdAgentRestInfoFull(agent, skillsRestInfo, queuesRestInfo, clientsRestInfo);

        return agentRestInfo;
    }

    private MetadataRestInfo addAgents(List<OpenAcdAgentRestInfoFull> agentsRestInfo, List<OpenAcdAgent> agents) {
        OpenAcdAgent agent;
        OpenAcdAgentRestInfoFull agentRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, agents.size());

        // create list of agent restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            agent = agents.get(index);
            agentRestInfo = createAgentRestInfo(agent);
            agentsRestInfo.add(agentRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    private List<OpenAcdSkillRestInfo> createSkillsRestInfo(OpenAcdAgent agent) {
        List<OpenAcdSkillRestInfo> skillsRestInfo;
        OpenAcdSkillRestInfo skillRestInfo;

        // create list of skill restinfos for single group
        Set<OpenAcdSkill> groupSkills = agent.getSkills();
        skillsRestInfo = new ArrayList<OpenAcdSkillRestInfo>(groupSkills.size());

        for (OpenAcdSkill groupSkill : groupSkills) {
            skillRestInfo = new OpenAcdSkillRestInfo(groupSkill);
            skillsRestInfo.add(skillRestInfo);
        }

        return skillsRestInfo;
    }

    private List<OpenAcdQueueRestInfo> createQueuesRestInfo(OpenAcdAgent agent) {
        List<OpenAcdQueueRestInfo> queuesRestInfo;
        OpenAcdQueueRestInfo queueRestInfo;

        // create list of queue restinfos for single group
        Set<OpenAcdQueue> groupQueues = agent.getQueues();
        queuesRestInfo = new ArrayList<OpenAcdQueueRestInfo>(groupQueues.size());

        for (OpenAcdQueue groupQueue : groupQueues) {
            queueRestInfo = new OpenAcdQueueRestInfo(groupQueue);
            queuesRestInfo.add(queueRestInfo);
        }

        return queuesRestInfo;
    }

    private List<OpenAcdClientRestInfo> createClientRestInfo(OpenAcdAgent agent) {
        List<OpenAcdClientRestInfo> clientsRestInfo;
        OpenAcdClientRestInfo clientRestInfo;

        // create list of queue restinfos for single group
        Set<OpenAcdClient> groupClients = agent.getClients();
        clientsRestInfo = new ArrayList<OpenAcdClientRestInfo>(groupClients.size());

        for (OpenAcdClient groupClient : groupClients) {
            clientRestInfo = new OpenAcdClientRestInfo(groupClient);
            clientsRestInfo.add(clientRestInfo);
        }

        return clientsRestInfo;

    }

    @SuppressWarnings("unchecked")
    private void sortAgents(List<OpenAcdAgent> agents) {
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
                Collections.sort(agents, new NameComparator());
            } else {
                Collections.sort(agents, new ReverseComparator(new NameComparator()));
            }
            break;

        case GROUP:
            if (sortForward) {
                Collections.sort(agents, new GroupComparator());
            } else {
                Collections.sort(agents, new ReverseComparator(new GroupComparator()));
            }
            break;

        case SECURITY:
            if (sortForward) {
                Collections.sort(agents, new SecurityComparator());
            } else {
                Collections.sort(agents, new ReverseComparator(new SecurityComparator()));
            }
            break;

        case USERNAME:
            if (sortForward) {
                Collections.sort(agents, new UserNameComparator());
            } else {
                Collections.sort(agents, new ReverseComparator(new UserNameComparator()));
            }
            break;

        case FIRSTNAME:
            if (sortForward) {
                Collections.sort(agents, new FirstNameComparator());
            } else {
                Collections.sort(agents, new ReverseComparator(new FirstNameComparator()));
            }
            break;

        case LASTNAME:
            if (sortForward) {
                Collections.sort(agents, new LastNameComparator());
            } else {
                Collections.sort(agents, new ReverseComparator(new LastNameComparator()));
            }
            break;

        case BRANCH:
            if (sortForward) {
                Collections.sort(agents, new BranchComparator());
            } else {
                Collections.sort(agents, new ReverseComparator(new BranchComparator()));
            }
            break;

        case EXTENSION:
            if (sortForward) {
                Collections.sort(agents, new ExtensionComparator());
            } else {
                Collections.sort(agents, new ReverseComparator(new ExtensionComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateAgent(OpenAcdAgent agent, OpenAcdAgentRestInfoFull agentRestInfo) throws ResourceException {
        OpenAcdAgentGroup agentGroup;

        agentGroup = getAgentGroup(agentRestInfo);
        agent.setGroup(agentGroup);

        agent.setSecurity(agentRestInfo.getSecurity());

        addLists(agent, agentRestInfo);
    }

    private OpenAcdAgent createOpenAcdAgent(OpenAcdAgentRestInfoFull agentRestInfo) throws ResourceException {
        OpenAcdAgent agent = new OpenAcdAgent();
        OpenAcdAgent duplicateAgent = null;
        User user;

        // set associated user
        user = m_coreContext.getUser(agentRestInfo.getUserId());

        // check if user is already assigned as agent
        duplicateAgent = m_openAcdContext.getAgentByUser(user);
        if (duplicateAgent != null) {
            throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST, "User " + user.getId()
                + " already assigned as agent.");
        }

        agent.setUser(user);

        updateAgent(agent, agentRestInfo);

        return agent;
    }

    private void addLists(OpenAcdAgent agent, OpenAcdAgentRestInfoFull agentRestInfo) {
        Set<OpenAcdSkill> skills = new LinkedHashSet<OpenAcdSkill>();
        List<OpenAcdSkillRestInfo> skillsRestInfo = agentRestInfo.getSkills();

        for (OpenAcdSkillRestInfo skillRestInfo : skillsRestInfo) {
            skills.add(m_openAcdContext.getSkillById(skillRestInfo.getId()));
        }

        Set<OpenAcdQueue> queues = new LinkedHashSet<OpenAcdQueue>();
        List<OpenAcdQueueRestInfo> queuesRestInfo = agentRestInfo.getQueues();

        for (OpenAcdQueueRestInfo queueRestInfo : queuesRestInfo) {
            queues.add(m_openAcdContext.getQueueById(queueRestInfo.getId()));
        }

        Set<OpenAcdClient> clients = new LinkedHashSet<OpenAcdClient>();
        List<OpenAcdClientRestInfo> clientsRestInfo = agentRestInfo.getClients();

        for (OpenAcdClientRestInfo clientRestInfo : clientsRestInfo) {
            clients.add(m_openAcdContext.getClientById(clientRestInfo.getId()));
        }

        agent.setSkills(skills);
        agent.setQueues(queues);
        agent.setClients(clients);
    }

    private OpenAcdAgentGroup getAgentGroup(OpenAcdAgentRestInfoFull agentRestInfo) {
        OpenAcdAgentGroup agentGroup;
        int groupId = 0;

        groupId = agentRestInfo.getGroupId();
        agentGroup = m_openAcdContext.getAgentGroupById(groupId);

        return agentGroup;
    }

    // Sorting comparator classes
    // --------------------------

    private static class NameComparator implements Comparator<OpenAcdAgent> {
        @Override
        public int compare(OpenAcdAgent item1, OpenAcdAgent item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class GroupComparator implements Comparator<OpenAcdAgent> {
        @Override
        public int compare(OpenAcdAgent item1, OpenAcdAgent item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getAgentGroup(), item2.getAgentGroup());
        }
    }

    private static class SecurityComparator implements Comparator<OpenAcdAgent> {
        public int compare(OpenAcdAgent item1, OpenAcdAgent item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getUser().getGroupsNames(), item2.getUser()
                    .getGroupsNames());
        }
    }

    private static class UserNameComparator implements Comparator<OpenAcdAgent> {
        public int compare(OpenAcdAgent item1, OpenAcdAgent item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getUser().getUserName(), item2.getUser()
                    .getUserName());
        }
    }

    private static class FirstNameComparator implements Comparator<OpenAcdAgent> {
        public int compare(OpenAcdAgent item1, OpenAcdAgent item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getFirstName(), item2.getFirstName());
        }
    }

    private static class LastNameComparator implements Comparator<OpenAcdAgent> {
        public int compare(OpenAcdAgent item1, OpenAcdAgent item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getLastName(), item2.getLastName());
        }
    }

    private static class BranchComparator implements Comparator<OpenAcdAgent> {
        public int compare(OpenAcdAgent item1, OpenAcdAgent item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getUser().getBranch().getName(), item2.getUser()
                    .getBranch().getName());
        }
    }

    private static class ExtensionComparator implements Comparator<OpenAcdAgent> {
        public int compare(OpenAcdAgent item1, OpenAcdAgent item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getUser().getExtension(true), item2.getUser()
                    .getExtension(true));
        }
    }

    // REST Representations
    // --------------------

    static class OpenAcdAgentsRepresentation extends XStreamRepresentation<OpenAcdAgentsBundleRestInfo> {

        public OpenAcdAgentsRepresentation(MediaType mediaType, OpenAcdAgentsBundleRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdAgentsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_AGENTBUNDLE, OpenAcdAgentsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_AGENT, OpenAcdAgentRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_GROUP, OpenAcdAgentGroupRestInfo.class);
            xstream.alias(ELEMENT_NAME_SKILL, OpenAcdSkillRestInfo.class);
            xstream.alias(ELEMENT_NAME_QUEUE, OpenAcdQueueRestInfo.class);
            xstream.alias(ELEMENT_NAME_CLIENT, OpenAcdClientRestInfo.class);
        }
    }

    static class OpenAcdAgentRepresentation extends XStreamRepresentation<OpenAcdAgentRestInfoFull> {

        public OpenAcdAgentRepresentation(MediaType mediaType, OpenAcdAgentRestInfoFull object) {
            super(mediaType, object);
        }

        public OpenAcdAgentRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_AGENT, OpenAcdAgentRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_GROUP, OpenAcdAgentGroupRestInfo.class);
            xstream.alias(ELEMENT_NAME_SKILL, OpenAcdSkillRestInfo.class);
            xstream.alias(ELEMENT_NAME_QUEUE, OpenAcdQueueRestInfo.class);
            xstream.alias(ELEMENT_NAME_CLIENT, OpenAcdClientRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdAgentsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<OpenAcdAgentRestInfoFull> m_agents;

        public OpenAcdAgentsBundleRestInfo(List<OpenAcdAgentRestInfoFull> agents, MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_agents = agents;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<OpenAcdAgentRestInfoFull> getAgents() {
            return m_agents;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }
}
