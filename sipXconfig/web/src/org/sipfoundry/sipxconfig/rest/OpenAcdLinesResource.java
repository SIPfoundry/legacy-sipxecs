/*
 *
 *  Copyright (C) 2012 PATLive, I. Wesson, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdLinesResource.java - A Restlet to read Skill data from OpenACD within SipXecs
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

import org.apache.commons.lang.StringUtils;
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
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchAction;
import org.sipfoundry.sipxconfig.openacd.OpenAcdClient;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdLine;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueue;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdClientRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdQueueRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

// OpenAcdLines are different
// --------------------------
// Lines is inconsistent with other OpenACD objects.
// OpenAcdContext does not contain functions for getLineById(), saveLine() or removeLine().
// It appears the OpenAcdExtension object is used for lines, and it has all the above functions
// so this API will appear slightly different than other APIs, although attempts have been made
// to preserve general structure.
public class OpenAcdLinesResource extends Resource {
    private static final String EMPTY_STRING = "";

    private static final String ELEMENT_NAME_LINEBUNDLE = "openacd-line";
    private static final String ELEMENT_NAME_LINE = "line";
    private static final String ELEMENT_NAME_ACTION = "action";

    private static final String SUPERVISION_TYPE_FS = "FS";
    private static final String SUPERVISION_TYPE_AGENT = "AGENT";
    private static final String SUPERVISION_TYPE_ACD = "ACD";

    private static final String ERROR_QUEUE_NOT_FOUND = "No queue with name: ";
    private static final String ERROR_CLIENT_NOT_FOUND = "No client with identity: ";

    private OpenAcdContext m_openAcdContext;
    private Form m_form;

    // use to define all possible sort fields
    enum SortField {
        NAME, DESCRIPTION, EXTENSION, DIDNUMBER, QUEUE, CLIENT, NONE;

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

    // GET - Retrieve all and single item
    // ----------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdLine line;
        OpenAcdLineRestInfo lineRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                line = (OpenAcdLine) m_openAcdContext.getExtensionById(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                lineRestInfo = createLineRestInfo(line);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new OpenAcdLineRepresentation(variant.getMediaType(), lineRestInfo);
        }

        // if not single, process request for list
        List<OpenAcdLine> lines = new ArrayList<OpenAcdLine>(m_openAcdContext.getLines());
        List<OpenAcdLineRestInfo> linesRestInfo = new ArrayList<OpenAcdLineRestInfo>();
        MetadataRestInfo metadataRestInfo;

        // sort groups if specified
        sortLines(lines);

        // set requested records and get resulting metadata
        metadataRestInfo = addLines(linesRestInfo, lines);

        // create final restinfo
        OpenAcdLinesBundleRestInfo linesBundleRestInfo =
                new OpenAcdLinesBundleRestInfo(linesRestInfo, metadataRestInfo);

        return new OpenAcdLinesRepresentation(variant.getMediaType(), linesBundleRestInfo);
    }

    // PUT - Update or Add single item
    // -------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get item from request body
        OpenAcdLineRepresentation representation = new OpenAcdLineRepresentation(entity);
        OpenAcdLineRestInfo lineRestInfo = representation.getObject();
        OpenAcdLine line;

        // validate input for update or create
        ValidationInfo validationInfo = validate(lineRestInfo);

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
                line = (OpenAcdLine) m_openAcdContext.getExtensionById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateLine(line, lineRestInfo);
                m_openAcdContext.saveExtension(line);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, line.getId());
            return;
        }

        // if not single, add new item
        try {
            line = createLine(lineRestInfo);
            m_openAcdContext.saveExtension(line);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, line.getId());
    }

    // DELETE - Delete single item
    // ---------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdLine line;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                line = (OpenAcdLine) m_openAcdContext.getExtensionById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            try {
                m_openAcdContext.deleteExtension(line);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, line.getId());
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
    private ValidationInfo validate(OpenAcdLineRestInfo restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String nameObject = "Name";

        String name = restInfo.getName();
        String didNumber = restInfo.getDIDNumber();
        String alias = restInfo.getAlias();

        // extension is only number if not regex, otherwise could be anything

        validationInfo.checkString(name, nameObject, StringConstraint.NOT_EMPTY);
        validationInfo.checkString(name, nameObject, StringConstraint.ONLY_LETTER_NUMBER_DASH_UNDERSCORE_SYMBOL);
        validationInfo.checkString(didNumber, "DID Number", StringConstraint.ONLY_NUMBER);
        validationInfo.checkString(alias, "Alias", StringConstraint.ONLY_NUMBER);

        return validationInfo;
    }

    // parses OpenAcdLine contents instead of just an id because line is so different
    private OpenAcdLineActionsBundleRestInfo createLineActionsBundleRestInfo(OpenAcdLine line) {
        OpenAcdLineActionsBundleRestInfo lineActionsBundleRestInfo;
        OpenAcdQueueRestInfo queueRestInfo;
        OpenAcdClientRestInfo clientRestInfo;
        List<OpenAcdLineActionRestInfo> customActions = new ArrayList<OpenAcdLineActionRestInfo>();
        OpenAcdLineActionRestInfo customActionRestInfo;

        OpenAcdQueue queue;
        String queueName = EMPTY_STRING;
        OpenAcdClient client;
        String clientIdentity = EMPTY_STRING;
        Boolean allowVoicemail = false;
        String allowVoicemailString = "false";
        Boolean isFsSet = false;
        Boolean isAgentSet = false;
        String answerSupervisionType = EMPTY_STRING;
        String welcomeMessage = EMPTY_STRING;

        List<FreeswitchAction> actions = line.getLineActions();
        for (FreeswitchAction action : actions) {
            String application = action.getApplication();
            String data = action.getData();

            if (StringUtils.equals(application, FreeswitchAction.PredefinedAction.answer.toString())) {
                isFsSet = true;
            } else if (StringUtils.contains(data, OpenAcdLine.ERLANG_ANSWER)) {
                isAgentSet = true;
            } else if (StringUtils.contains(data, OpenAcdLine.Q)) {
                queueName = StringUtils.removeStart(data, OpenAcdLine.Q);
            } else if (StringUtils.contains(data, OpenAcdLine.BRAND)) {
                clientIdentity = StringUtils.removeStart(data, OpenAcdLine.BRAND);
            } else if (StringUtils.contains(data, OpenAcdLine.ALLOW_VOICEMAIL)) {
                allowVoicemailString = StringUtils.removeStart(data, OpenAcdLine.ALLOW_VOICEMAIL);
            } else if (StringUtils.equals(application, FreeswitchAction.PredefinedAction.playback.toString())) {
                // web ui only displays filename and appends audio directory
                // welcomeMessage = StringUtils.removeStart(data,
                // m_openAcdContext.getSettings().getAudioDirectory() + "/");
                welcomeMessage = data;
            } else {
                customActionRestInfo = new OpenAcdLineActionRestInfo(action);
                customActions.add(customActionRestInfo);
            }
        }

        queue = m_openAcdContext.getQueueByName(queueName);
        queueRestInfo = new OpenAcdQueueRestInfo(queue);

        client = m_openAcdContext.getClientByIdentity(clientIdentity);
        clientRestInfo = new OpenAcdClientRestInfo(client);

        allowVoicemail = Boolean.parseBoolean(allowVoicemailString);

        if (isFsSet) {
            answerSupervisionType = SUPERVISION_TYPE_FS;
        } else if (isAgentSet) {
            answerSupervisionType = SUPERVISION_TYPE_AGENT;
        } else {
            answerSupervisionType = SUPERVISION_TYPE_ACD;
        }

        lineActionsBundleRestInfo =
                new OpenAcdLineActionsBundleRestInfo(queueRestInfo, clientRestInfo, allowVoicemail, customActions,
                        answerSupervisionType, welcomeMessage);

        return lineActionsBundleRestInfo;
    }

    private OpenAcdLineRestInfo createLineRestInfo(OpenAcdLine line) {
        OpenAcdLineRestInfo lineRestInfo;
        OpenAcdLineActionsBundleRestInfo lineActionsBundleRestInfo;

        lineActionsBundleRestInfo = createLineActionsBundleRestInfo(line);
        lineRestInfo = new OpenAcdLineRestInfo(line, lineActionsBundleRestInfo);

        return lineRestInfo;
    }

    private MetadataRestInfo addLines(List<OpenAcdLineRestInfo> linesRestInfo, List<OpenAcdLine> lines) {
        OpenAcdLine line;
        OpenAcdLineRestInfo lineRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, lines.size());

        // create list of line restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            line = lines.get(index);
            lineRestInfo = createLineRestInfo(line);
            linesRestInfo.add(lineRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortLines(List<OpenAcdLine> lines) {
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
                Collections.sort(lines, new NameComparator());
            } else {
                Collections.sort(lines, new ReverseComparator(new NameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(lines, new DescriptionComparator());
            } else {
                Collections.sort(lines, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        case EXTENSION:
            if (sortForward) {
                Collections.sort(lines, new ExtensionComparator());
            } else {
                Collections.sort(lines, new ReverseComparator(new ExtensionComparator()));
            }
            break;

        case DIDNUMBER:
            if (sortForward) {
                Collections.sort(lines, new DIDNumberComparator());
            } else {
                Collections.sort(lines, new ReverseComparator(new DIDNumberComparator()));
            }
            break;

        case QUEUE:
            if (sortForward) {
                Collections.sort(lines, new QueueComparator());
            } else {
                Collections.sort(lines, new ReverseComparator(new QueueComparator()));
            }
            break;

        case CLIENT:
            if (sortForward) {
                Collections.sort(lines, new ClientComparator());
            } else {
                Collections.sort(lines, new ReverseComparator(new ClientComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateLine(OpenAcdLine line, OpenAcdLineRestInfo lineRestInfo) throws ResourceException {
        OpenAcdQueue queue;
        OpenAcdClient client;

        line.setName(lineRestInfo.getName());
        line.setDid(lineRestInfo.getDIDNumber());
        line.setDescription(lineRestInfo.getDescription());
        line.setAlias(lineRestInfo.getAlias());

        // set standard actions
        line.getNumberCondition().getActions().clear();

        // answer supervision type is an integer code from OpenAcdLine
        line.getNumberCondition().addAction(
                OpenAcdLine.createAnswerAction(getAnswerSupervisionCode(lineRestInfo.getActions()
                        .getAnswerSupervisionType())));
        line.getNumberCondition().addAction(
                OpenAcdLine.createVoicemailAction(lineRestInfo.getActions().getAllowVoicemail()));

        // check queue name
        queue = m_openAcdContext.getQueueByName(lineRestInfo.getActions().getQueue().getName());
        if (queue == null) {
            throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST, ERROR_QUEUE_NOT_FOUND
                + lineRestInfo.getActions().getQueue().getName());
        }

        line.getNumberCondition().addAction(
                line.createQueueAction(m_openAcdContext.getQueueById(lineRestInfo.getActions().getQueue().getId())));

        // check client identity
        client = m_openAcdContext.getClientByIdentity(lineRestInfo.getActions().getClient().getIdentity());
        if (client == null) {
            throw new ResourceException(Status.CLIENT_ERROR_BAD_REQUEST, ERROR_CLIENT_NOT_FOUND
                + lineRestInfo.getActions().getClient().getIdentity());
        }

        line.getNumberCondition().addAction(
                line.createClientAction(m_openAcdContext.getClientById((lineRestInfo.getActions().getClient()
                        .getId()))));

        // web ui only displays filename and appends audio directory
        // line.getNumberCondition().addAction(OpenAcdLine.createPlaybackAction(
        // m_openAcdContext.getSettings().getAudioDirectory() + "/" +
        // lineRestInfo.getActions().getWelcomeMessage()));
        line.getNumberCondition().addAction(
                OpenAcdLine.createPlaybackAction(lineRestInfo.getActions().getWelcomeMessage()));

        // set custom actions
        for (OpenAcdLineActionRestInfo actionRestInfo : lineRestInfo.getActions().getCustomActions()) {
            line.getNumberCondition().addAction(
                    OpenAcdLine.createAction(actionRestInfo.getApplication(), actionRestInfo.getData()));
        }

        // "Expression" is the extension number, which may be a regular expression if regex is set
        line.getNumberCondition().setExpression(lineRestInfo.getExtension());
        line.getNumberCondition().setRegex(lineRestInfo.getRegex());
    }

    private OpenAcdLine createLine(OpenAcdLineRestInfo lineRestInfo) throws ResourceException {
        // special steps to obtain new line (cannot just "new")
        OpenAcdLine line = m_openAcdContext.newOpenAcdLine();
        line.addCondition(OpenAcdLine.createLineCondition());

        updateLine(line, lineRestInfo);

        return line;
    }

    private Integer getAnswerSupervisionCode(String answerSupervisionType) {
        Integer answerSupervisionCode;

        if (StringUtils.equalsIgnoreCase(answerSupervisionType, SUPERVISION_TYPE_FS)) {
            answerSupervisionCode = OpenAcdLine.FS;
        } else if (StringUtils.equalsIgnoreCase(answerSupervisionType, SUPERVISION_TYPE_AGENT)) {
            answerSupervisionCode = OpenAcdLine.AGENT;
        } else {
            answerSupervisionCode = OpenAcdLine.ACD;
        }

        return answerSupervisionCode;
    }

    private static String getQueueName(OpenAcdLine line) {
        String queueName = EMPTY_STRING;
        List<FreeswitchAction> actions = line.getLineActions();

        for (FreeswitchAction action : actions) {
            String data = action.getData();

            if (StringUtils.contains(data, OpenAcdLine.Q)) {
                queueName = StringUtils.removeStart(data, OpenAcdLine.Q);
            }
        }

        return queueName;
    }

    private static String getClientIdentity(OpenAcdLine line) {
        String clientIdentity = EMPTY_STRING;
        List<FreeswitchAction> actions = line.getLineActions();

        for (FreeswitchAction action : actions) {
            String data = action.getData();

            if (StringUtils.contains(data, OpenAcdLine.BRAND)) {
                clientIdentity = StringUtils.removeStart(data, OpenAcdLine.BRAND);
            }
        }

        return clientIdentity;
    }

    // Sorting comparator classes
    // --------------------------

    private static class NameComparator implements Comparator<OpenAcdLine> {
        @Override
        public int compare(OpenAcdLine item1, OpenAcdLine item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class DescriptionComparator implements Comparator<OpenAcdLine> {
        @Override
        public int compare(OpenAcdLine item1, OpenAcdLine item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    private static class ExtensionComparator implements Comparator<OpenAcdLine> {
        @Override
        public int compare(OpenAcdLine item1, OpenAcdLine item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getExtension(), item2.getExtension());
        }
    }

    private static class DIDNumberComparator implements Comparator<OpenAcdLine> {
        @Override
        public int compare(OpenAcdLine item1, OpenAcdLine item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDid(), item2.getDid());
        }
    }

    private static class QueueComparator implements Comparator<OpenAcdLine> {
        @Override
        public int compare(OpenAcdLine item1, OpenAcdLine item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(getQueueName(item1), getQueueName(item2));
        }
    }

    private static class ClientComparator implements Comparator<OpenAcdLine> {
        @Override
        public int compare(OpenAcdLine item1, OpenAcdLine item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(getClientIdentity(item1), getClientIdentity(item2));
        }
    }

    // REST Representations
    // --------------------

    static class OpenAcdLinesRepresentation extends XStreamRepresentation<OpenAcdLinesBundleRestInfo> {

        public OpenAcdLinesRepresentation(MediaType mediaType, OpenAcdLinesBundleRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdLinesRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_LINEBUNDLE, OpenAcdLinesBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_LINE, OpenAcdLineRestInfo.class);
            xstream.alias(ELEMENT_NAME_ACTION, OpenAcdLineActionRestInfo.class);
        }
    }

    static class OpenAcdLineRepresentation extends XStreamRepresentation<OpenAcdLineRestInfo> {

        public OpenAcdLineRepresentation(MediaType mediaType, OpenAcdLineRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdLineRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_LINE, OpenAcdLineRestInfo.class);
            xstream.alias(ELEMENT_NAME_ACTION, OpenAcdLineActionRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdLinesBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<OpenAcdLineRestInfo> m_lines;

        public OpenAcdLinesBundleRestInfo(List<OpenAcdLineRestInfo> lines, MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_lines = lines;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<OpenAcdLineRestInfo> getLines() {
            return m_lines;
        }
    }

    static class OpenAcdLineRestInfo {
        private final int m_id;
        private final String m_name;
        private final String m_description;
        private final String m_extension;
        private final boolean m_regex;
        private final String m_didnumber;
        private final String m_alias;
        private final OpenAcdLineActionsBundleRestInfo m_actions;

        public OpenAcdLineRestInfo(OpenAcdLine line, OpenAcdLineActionsBundleRestInfo lineActionsRestInfo) {
            m_id = line.getId();
            m_name = line.getName();
            m_description = line.getDescription();
            m_extension = line.getExtension();
            m_regex = line.getRegex();
            m_didnumber = line.getDid();
            m_alias = line.getAlias();
            m_actions = lineActionsRestInfo;
        }

        public int getId() {
            return m_id;
        }

        public String getName() {
            return m_name;
        }

        public String getDescription() {
            return m_description;
        }

        public String getExtension() {
            return m_extension;
        }

        public boolean getRegex() {
            return m_regex;
        }

        public String getDIDNumber() {
            return m_didnumber;
        }

        public String getAlias() {
            return m_alias;
        }

        public OpenAcdLineActionsBundleRestInfo getActions() {
            return m_actions;
        }
    }

    static class OpenAcdLineActionRestInfo {
        private final String m_application;
        private final String m_data;

        public OpenAcdLineActionRestInfo(FreeswitchAction action) {
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

    static class OpenAcdLineActionsBundleRestInfo {
        // standard (required?) actions
        private final OpenAcdQueueRestInfo m_queue;
        private final OpenAcdClientRestInfo m_client;
        private final boolean m_allowVoicemail;
        private final String m_answerSupervisionType;
        private final String m_welcomeMessage;

        // additional (custom) actions
        private final List<OpenAcdLineActionRestInfo> m_customActions;

        public OpenAcdLineActionsBundleRestInfo(OpenAcdQueueRestInfo queueRestInfo,
                OpenAcdClientRestInfo clientRestInfo, boolean allowVoicemail,
                List<OpenAcdLineActionRestInfo> customActions, String answerSupervisionType, String welcomeMessage) {
            m_queue = queueRestInfo;
            m_client = clientRestInfo;
            m_allowVoicemail = allowVoicemail;
            m_customActions = customActions;
            m_answerSupervisionType = answerSupervisionType;
            m_welcomeMessage = welcomeMessage;
        }

        public OpenAcdQueueRestInfo getQueue() {
            return m_queue;
        }

        public OpenAcdClientRestInfo getClient() {
            return m_client;
        }

        public boolean getAllowVoicemail() {
            return m_allowVoicemail;
        }

        public String getAnswerSupervisionType() {
            return m_answerSupervisionType;
        }

        public String getWelcomeMessage() {
            return m_welcomeMessage;
        }

        public List<OpenAcdLineActionRestInfo> getCustomActions() {
            return m_customActions;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

}
