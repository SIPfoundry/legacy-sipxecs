/*
 *
 *  Copyright (C) 2012 PATLive, I. Wesson, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdClientsResource.java - A Restlet to read Skill data from OpenACD within SipXecs
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
import java.util.List;

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
import org.sipfoundry.sipxconfig.openacd.OpenAcdClient;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdClientRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class OpenAcdClientsResource extends Resource {
    private static final String ELEMENT_NAME_CLIENTBUNDLE = "openacd-client";
    private static final String ELEMENT_NAME_CLIENT = "client";

    private OpenAcdContext m_openAcdContext;
    private Form m_form;

    // use to define all possible sort fields
    enum SortField {
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

    @Override
    public boolean allowDelete() {
        return true;
    }

    // GET - Retrieve all and single Client
    // ------------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdClient client;
        OpenAcdClientRestInfo clientRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                client = m_openAcdContext.getClientById(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                clientRestInfo = createClientRestInfo(client);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new OpenAcdClientRepresentation(variant.getMediaType(), clientRestInfo);
        }

        // if not single, process request for list
        List<OpenAcdClient> clients = m_openAcdContext.getClients();
        List<OpenAcdClientRestInfo> clientsRestInfo = new ArrayList<OpenAcdClientRestInfo>();
        MetadataRestInfo metadataRestInfo;

        // sort groups if specified
        sortClients(clients);

        // set requested records and get resulting metadata
        metadataRestInfo = addClients(clientsRestInfo, clients);

        // create final restinfo
        OpenAcdClientsBundleRestInfo clientsBundleRestInfo =
                new OpenAcdClientsBundleRestInfo(clientsRestInfo, metadataRestInfo);

        return new OpenAcdClientsRepresentation(variant.getMediaType(), clientsBundleRestInfo);
    }

    // PUT - Update or Add single Client
    // ---------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get item from request body
        OpenAcdClientRepresentation representation = new OpenAcdClientRepresentation(entity);
        OpenAcdClientRestInfo clientRestInfo = representation.getObject();
        OpenAcdClient client;

        // validate input for update or create
        ValidationInfo validationInfo = validate(clientRestInfo);

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
                client = m_openAcdContext.getClientById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateClient(client, clientRestInfo);
                m_openAcdContext.saveClient(client);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, client.getId());
            return;
        }

        // if not single, add new item
        try {
            client = createClient(clientRestInfo);
            m_openAcdContext.saveClient(client);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, client.getId());
    }

    // DELETE - Delete single Client
    // -----------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdClient client;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                client = m_openAcdContext.getClientById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            try {
                m_openAcdContext.deleteClient(client);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, client.getId());
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
    private ValidationInfo validate(OpenAcdClientRestInfo restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String identityObject = "Identity";

        String name = restInfo.getName();
        String identity = restInfo.getIdentity();

        validationInfo.checkString(name, "Name", StringConstraint.NOT_EMPTY);
        validationInfo.checkString(identity, identityObject, StringConstraint.NOT_EMPTY);
        validationInfo.checkString(identity, identityObject, StringConstraint.ONLY_LETTER_NUMBER_DASH_UNDERSCORE);

        return validationInfo;
    }

    private OpenAcdClientRestInfo createClientRestInfo(OpenAcdClient client) {
        OpenAcdClientRestInfo clientRestInfo;

        clientRestInfo = new OpenAcdClientRestInfo(client);

        return clientRestInfo;
    }

    private MetadataRestInfo addClients(List<OpenAcdClientRestInfo> clientsRestInfo, List<OpenAcdClient> clients) {
        OpenAcdClient client;
        OpenAcdClientRestInfo clientRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, clients.size());

        // create list of client restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            client = clients.get(index);
            clientRestInfo = createClientRestInfo(client);
            clientsRestInfo.add(clientRestInfo);
        }

        // create metadata about results
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortClients(List<OpenAcdClient> clients) {
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
                Collections.sort(clients, new NameComparator());
            } else {
                Collections.sort(clients, new ReverseComparator(new NameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(clients, new DescriptionComparator());
            } else {
                Collections.sort(clients, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateClient(OpenAcdClient client, OpenAcdClientRestInfo clientRestInfo) throws ResourceException {
        client.setName(clientRestInfo.getName());
        client.setDescription(clientRestInfo.getDescription());
    }

    private OpenAcdClient createClient(OpenAcdClientRestInfo clientRestInfo) throws ResourceException {
        OpenAcdClient client = new OpenAcdClient();

        // only set identity when create (cannot update)
        client.setIdentity(clientRestInfo.getIdentity());

        updateClient(client, clientRestInfo);

        return client;
    }

    // Sorting comparator classes
    // --------------------------

    private static class NameComparator implements Comparator<OpenAcdClient> {
        @Override
        public int compare(OpenAcdClient item1, OpenAcdClient item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class DescriptionComparator implements Comparator<OpenAcdClient> {
        @Override
        public int compare(OpenAcdClient item1, OpenAcdClient item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    // REST Representations
    // --------------------

    static class OpenAcdClientsRepresentation extends XStreamRepresentation<OpenAcdClientsBundleRestInfo> {

        public OpenAcdClientsRepresentation(MediaType mediaType, OpenAcdClientsBundleRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdClientsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_CLIENTBUNDLE, OpenAcdClientsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_CLIENT, OpenAcdClientRestInfo.class);
        }
    }

    static class OpenAcdClientRepresentation extends XStreamRepresentation<OpenAcdClientRestInfo> {

        public OpenAcdClientRepresentation(MediaType mediaType, OpenAcdClientRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdClientRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_CLIENT, OpenAcdClientRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdClientsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<OpenAcdClientRestInfo> m_clients;

        public OpenAcdClientsBundleRestInfo(List<OpenAcdClientRestInfo> clients, MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_clients = clients;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<OpenAcdClientRestInfo> getClients() {
            return m_clients;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

}
