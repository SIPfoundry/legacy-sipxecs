/*
 *
 *  Copyright (C) 2012 PATLive, D. Waseem, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdDialStringsResource.java - A Restlet to read DialString data from OpenACD within SipXecs
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
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.openacd.OpenAcdCommand;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class OpenAcdDialStringsResource extends Resource {

    private static final String ELEMENT_NAME_DIALSTRINGBUNDLE = "openacd-dial-string";
    private static final String ELEMENT_NAME_DIALSTRING = "dialString";
    private static final String ELEMENT_NAME_ACTION = "action";

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

    // GET - Retrieve all and single Dial Strings
    // ------------------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdCommand dialString;
        OpenAcdDialStringRestInfo dialStringRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                dialString = (OpenAcdCommand) m_openAcdContext.getExtensionById(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                dialStringRestInfo = createDialStringRestInfo(dialString);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new OpenAcdDialStringRepresentation(variant.getMediaType(), dialStringRestInfo);
        }

        // if not single, process request for list
        List<OpenAcdCommand> dialStrings = new ArrayList<OpenAcdCommand>(m_openAcdContext.getCommands());
        List<OpenAcdDialStringRestInfo> dialStringsRestInfo = new ArrayList<OpenAcdDialStringRestInfo>();
        MetadataRestInfo metadataRestInfo;

        // sort groups if specified
        sortDialStrings(dialStrings);

        // set requested records and get resulting metadata
        metadataRestInfo = addDialStrings(dialStringsRestInfo, dialStrings);

        // create final restinfo
        OpenAcdDialStringsBundleRestInfo dialStringsBundleRestInfo =
                new OpenAcdDialStringsBundleRestInfo(dialStringsRestInfo, metadataRestInfo);

        return new OpenAcdDialStringsRepresentation(variant.getMediaType(), dialStringsBundleRestInfo);
    }

    // PUT - Update or Add single Dial String
    // --------------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get from request body
        OpenAcdDialStringRepresentation representation = new OpenAcdDialStringRepresentation(entity);
        OpenAcdDialStringRestInfo dialStringRestInfo = representation.getObject();
        OpenAcdCommand dialString;

        ValidationInfo validationInfo = validate(dialStringRestInfo);

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
                dialString = (OpenAcdCommand) m_openAcdContext.getExtensionById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateDialString(dialString, dialStringRestInfo);
                m_openAcdContext.saveExtension(dialString);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, dialString.getId());
            return;
        }

        // if not single, add new item
        try {
            dialString = createDialString(dialStringRestInfo);
            m_openAcdContext.saveExtension(dialString);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, dialString.getId());
    }

    // DELETE - Delete single Dial String
    // ----------------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdCommand dialString;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                dialString = (OpenAcdCommand) m_openAcdContext.getExtensionById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            try {
                m_openAcdContext.deleteExtension(dialString);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, dialString.getId());
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
    private ValidationInfo validate(OpenAcdDialStringRestInfo restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String name = restInfo.getName();

        validationInfo.checkString(name, "Name", StringConstraint.NOT_EMPTY);

        return validationInfo;
    }

    private OpenAcdDialStringRestInfo createDialStringRestInfo(OpenAcdCommand dialString) {
        OpenAcdDialStringRestInfo dialStringRestInfo;
        List<OpenAcdDialStringActionRestInfo> dialStringActionRestInfo;

        dialStringActionRestInfo = createDialStringActionRestInfo(dialString);
        dialStringRestInfo = new OpenAcdDialStringRestInfo(dialString, dialStringActionRestInfo);

        return dialStringRestInfo;

    }

    private List<OpenAcdDialStringActionRestInfo> createDialStringActionRestInfo(OpenAcdCommand dialString) {
        OpenAcdDialStringActionRestInfo dialStringActionRestInfo;
        List<OpenAcdDialStringActionRestInfo> customActions = new ArrayList<OpenAcdDialStringActionRestInfo>();

        List<FreeswitchAction> actions = dialString.getLineActions();

        for (FreeswitchAction action : actions) {
            dialStringActionRestInfo = new OpenAcdDialStringActionRestInfo(action);
            customActions.add(dialStringActionRestInfo);
        }

        return customActions;
    }

    private MetadataRestInfo addDialStrings(List<OpenAcdDialStringRestInfo> dialStringsRestInfo,
            List<OpenAcdCommand> dialStrings) {
        OpenAcdCommand dialString;
        OpenAcdDialStringRestInfo dialStringRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, dialStrings.size());

        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            dialString = dialStrings.get(index);
            dialStringRestInfo = createDialStringRestInfo(dialString);
            dialStringsRestInfo.add(dialStringRestInfo);
        }

        // create metadata about results
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortDialStrings(List<OpenAcdCommand> dialStrings) {
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
                Collections.sort(dialStrings, new NameComparator());
            } else {
                Collections.sort(dialStrings, new ReverseComparator(new NameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(dialStrings, new DescriptionComparator());
            } else {
                Collections.sort(dialStrings, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateDialString(OpenAcdCommand dialString, OpenAcdDialStringRestInfo dialStringRestInfo) {
        dialString.setName(dialStringRestInfo.getName());
        dialString.setEnabled(dialStringRestInfo.getEnabled());
        dialString.setDescription(dialStringRestInfo.getDescription());
        dialString.getNumberCondition().setExpression(dialStringRestInfo.getExtension());

        for (OpenAcdDialStringActionRestInfo actionRestInfo : dialStringRestInfo.getActions()) {
            dialString.getNumberCondition().addAction(
                    OpenAcdCommand.createAction(actionRestInfo.getApplication(), actionRestInfo.getApplication()));
        }
    }

    private OpenAcdCommand createDialString(OpenAcdDialStringRestInfo dialStringRestInfo) throws ResourceException {
        OpenAcdCommand dialString = m_openAcdContext.newOpenAcdCommand();
        dialString.addCondition(OpenAcdCommand.createLineCondition());

        updateDialString(dialString, dialStringRestInfo);

        return dialString;
    }

    // Sorting comparator classes
    // --------------------------

    private static class NameComparator implements Comparator<OpenAcdCommand> {
        @Override
        public int compare(OpenAcdCommand item1, OpenAcdCommand item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class DescriptionComparator implements Comparator<OpenAcdCommand> {
        @Override
        public int compare(OpenAcdCommand item1, OpenAcdCommand item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    // REST Representations
    // --------------------

    static class OpenAcdDialStringsRepresentation extends XStreamRepresentation<OpenAcdDialStringsBundleRestInfo> {

        public OpenAcdDialStringsRepresentation(MediaType mediaType, OpenAcdDialStringsBundleRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdDialStringsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_DIALSTRINGBUNDLE, OpenAcdDialStringsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_DIALSTRING, OpenAcdDialStringRestInfo.class);
            xstream.alias(ELEMENT_NAME_ACTION, OpenAcdDialStringActionRestInfo.class);
        }
    }

    static class OpenAcdDialStringRepresentation extends XStreamRepresentation<OpenAcdDialStringRestInfo> {

        public OpenAcdDialStringRepresentation(MediaType mediaType,
                OpenAcdDialStringRestInfo dialStringsBundleRestInfo) {
            super(mediaType, dialStringsBundleRestInfo);
        }

        public OpenAcdDialStringRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_DIALSTRING, OpenAcdDialStringRestInfo.class);
            xstream.alias(ELEMENT_NAME_ACTION, OpenAcdDialStringActionRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdDialStringsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<OpenAcdDialStringRestInfo> m_dialStrings;

        public OpenAcdDialStringsBundleRestInfo(List<OpenAcdDialStringRestInfo> dialStrings,
                MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_dialStrings = dialStrings;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<OpenAcdDialStringRestInfo> getDialStrings() {
            return m_dialStrings;
        }
    }

    static class OpenAcdDialStringRestInfo {
        private final int m_id;
        private final String m_name;
        private final boolean m_enabled;
        private final String m_description;
        private final String m_extension;
        private final List<OpenAcdDialStringActionRestInfo> m_actions;

        public OpenAcdDialStringRestInfo(OpenAcdCommand dial,
                List<OpenAcdDialStringActionRestInfo> dialStringActionsRestInfo) {
            m_id = dial.getId();
            m_name = dial.getName();
            m_enabled = dial.isEnabled();
            m_description = dial.getDescription();
            m_extension = dial.getExtension();
            m_actions = dialStringActionsRestInfo;
        }

        public int getId() {
            return m_id;
        }

        public String getName() {
            return m_name;
        }

        public boolean getEnabled() {
            return m_enabled;
        }

        public String getDescription() {
            return m_description;
        }

        public String getExtension() {
            return m_extension;
        }

        public List<OpenAcdDialStringActionRestInfo> getActions() {
            return m_actions;
        }
    }

    static class OpenAcdDialStringActionRestInfo {
        private final String m_application;
        private final String m_data;

        public OpenAcdDialStringActionRestInfo(FreeswitchAction action) {
            m_application = action.getApplication();
            m_data = action.getData();
        }

        public String getApplication() {
            return m_application;
        }

        public String getData() {
            return m_data;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }
}
