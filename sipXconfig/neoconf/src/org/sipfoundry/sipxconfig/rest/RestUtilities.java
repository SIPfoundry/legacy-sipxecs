/*
 *
 *  Copyright (C) 2012 PATLive, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdUtilities.java - Support functionality for OpenAcd Restlets
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

import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_VALIDATION_FAILED;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;

import org.apache.commons.lang.StringUtils;
import org.restlet.data.ClientInfo;
import org.restlet.data.Form;
import org.restlet.data.Language;
import org.restlet.data.MediaType;
import org.restlet.data.Preference;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.data.Status;
import org.restlet.resource.DomRepresentation;
import org.restlet.resource.Representation;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.setting.Group;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

public final class RestUtilities {
    public static final String EMPTY_STRING = "";
    public static final String COLON = ":";
    public static final String ID = "id";

    public static final String REQUEST_ATTRIBUTE_ID = ID;
    public static final String REQUEST_ATTRIBUTE_NAME = "name";
    public static final String REQUEST_ATTRIBUTE_BRANCH = "branch";
    public static final String REQUEST_ATTRIBUTE_IDLIST = "ids";

    public static final String ELEMENT_DATA = "data";
    public static final String ELEMENT_ID = ID;
    public static final String ELEMENT_RESPONSE = "response";
    public static final String ELEMENT_ADDITIONAL_MESSAGE = "additionalMessage";
    public static final String ELEMENT_STRING = "string";
    public static final String ELEMENT_CODE = "code";
    public static final String ELEMENT_MESSAGE = "message";
    public static final String ELEMENT_OBJECT = "object";

    public static enum ResponseCode {
        SUCCESS, SUCCESS_CREATED, SUCCESS_UPDATED, SUCCESS_DELETED, ERROR_MISSING_ID, ERROR_OBJECT_NOT_FOUND,
        ERROR_ID_INVALID, ERROR_VALIDATION_FAILED, ERROR_UPDATE_FAILED, ERROR_CREATE_FAILED, ERROR_READ_FAILED,
        ERROR_DELETE_FAILED, ERROR_REFERENCE_EXISTS
    }

    public static enum ResponseElements {
        NONE, ADDITIONAL_MESSAGE, ID, ID_ADDITIONAL_MESSAGE, STRING_ADDITIONAL_MESSAGE
    }

    private RestUtilities() {
        // hide default constructor
    }

    /**
     * TODO: Move this to common rest util package
     */
    public static Locale getLocale(Request request) {
        ClientInfo ci = request.getClientInfo();
        if (ci != null) {
            List<Preference<Language>> langs = ci.getAcceptedLanguages();
            if (langs != null && langs.size() > 0) {
                Language lmeta = langs.get(0).getMetadata();
                if (lmeta != null && lmeta.getName() != null) {
                    // Java 1.7 only
                    //   Locale.forLanguageTag(lmeta.getName());
                    return forLanguageTag(lmeta.getName());
                }
            }
        }
        return Locale.ENGLISH;
    }

    public static Locale forLanguageTag(String id) {
        String[] segments = StringUtils.split(id, '-');
        switch (segments.length) {
        case 3:
            return new Locale(segments[0], segments[1], segments[2]);
        case 2:
            return new Locale(segments[0], segments[1]);
        default:
        case 1:
            return new Locale(id);
        }
    }

    // Input parameter conversion functions
    // ------------------------------------

    public static IntParameterInfo getIntFromAttribute(Request request, String attributeName) {
        IntParameterInfo parameterInfo;
        String stringFromAttribute;

        stringFromAttribute = (String) request.getAttributes().get(attributeName);
        parameterInfo = getIntParameterInfoFromString(stringFromAttribute);

        return parameterInfo;
    }

    public static StringParameterInfo getStringFromAttribute(Request request, String attributeName) {
        StringParameterInfo parameterInfo;
        String stringFromAttribute;

        stringFromAttribute = (String) request.getAttributes().get(attributeName);
        parameterInfo = getStringParameterInfo(stringFromAttribute);

        return parameterInfo;
    }

    public static IntParameterInfo getIntFromQueryString(Form form, String parameterName) {
        IntParameterInfo parameterInfo;
        String stringFromParameter;

        stringFromParameter = form.getFirstValue(parameterName);
        parameterInfo = getIntParameterInfoFromString(stringFromParameter);

        return parameterInfo;
    }

    public static IntListParameterInfo getIntListFromQueryString(Form form, String parameterName) {
        IntListParameterInfo parameterInfo;
        String stringFromParameter;

        stringFromParameter = form.getFirstValue(parameterName);
        parameterInfo = getIntListParameterInfoFromString(stringFromParameter);

        return parameterInfo;
    }

    private static IntParameterInfo getIntParameterInfoFromString(String intString) {
        IntParameterInfo parameterInfo = new IntParameterInfo();
        int intFromString = 0;

        // is there any parameter value at all?
        if (intString != null) {
            parameterInfo.setExists(true);
            parameterInfo.setValueString(intString);

            // attempt to parse string provided as an integer
            parameterInfo.setValid(true);
            try {
                intFromString = Integer.parseInt(intString);
            } catch (Exception exception) {
                parameterInfo.setValid(false);
            }
        } else {
            parameterInfo.setExists(false);
            parameterInfo.setValid(false);
        }

        // was value valid? (if no parameter then invalid)
        if (parameterInfo.getValid()) {
            parameterInfo.setValue(intFromString);
        }

        return parameterInfo;
    }

    private static StringParameterInfo getStringParameterInfo(String sourceString) {
        StringParameterInfo parameterInfo = new StringParameterInfo();

        // is there any parameter value at all?
        if ((sourceString != null) && (!sourceString.isEmpty())) {
            parameterInfo.setExists(true);
            parameterInfo.setValid(true);
            parameterInfo.setValue(sourceString);
        } else {
            parameterInfo.setExists(false);
            parameterInfo.setValid(false);
            parameterInfo.setValue(EMPTY_STRING);
        }

        return parameterInfo;
    }

    private static IntListParameterInfo getIntListParameterInfoFromString(String intListString) {
        IntListParameterInfo parameterInfo = new IntListParameterInfo();
        int intFromString = 0;
        List<Integer> intList = new ArrayList<Integer>();

        // is there any parameter value at all?
        if (intListString != null) {
            parameterInfo.setExists(true);
            parameterInfo.setListString(intListString);

            // attempt to parse string provided as a comma-delimited list of integer
            parameterInfo.setValid(true);
            try {
                String[] stringArray = intListString.split(",");

                for (String stringInt : stringArray) {
                    intFromString = Integer.parseInt(stringInt);
                    intList.add(intFromString);
                }
            } catch (Exception exception) {
                parameterInfo.setValid(false);
            }
        } else {
            parameterInfo.setExists(false);
            parameterInfo.setValid(false);
        }

        // was value valid? (if no parameter then invalid)
        if (parameterInfo.getValid()) {
            parameterInfo.setIntList(intList);
        }

        return parameterInfo;
    }

    // List filtering functions
    // ------------------------

    public static PaginationInfo calculatePagination(Form form, int totalResults) {
        PaginationInfo paginationInfo = new PaginationInfo();
        paginationInfo.setTotalResults(totalResults);

        // must specify both PageNumber and ResultsPerPage together
        String pageNumberString = form.getFirstValue("page");
        String resultsPerPageString = form.getFirstValue("pagesize");

        // attempt to parse pagination values from request
        try {
            paginationInfo.setPageNumber(Integer.parseInt(pageNumberString));
            paginationInfo.setResultsPerPage(Integer.parseInt(resultsPerPageString));
        } catch (Exception exception) {
            // default 0 for nothing
            paginationInfo.setPageNumber(0);
            paginationInfo.setResultsPerPage(0);
        }

        // check for outrageous values or lack of parameters
        if ((paginationInfo.getPageNumber() < 1) || (paginationInfo.getResultsPerPage() < 1)) {
            paginationInfo.setPageNumber(0);
            paginationInfo.setResultsPerPage(0);
            paginationInfo.setPaginate(false);
        } else {
            paginationInfo.setPaginate(true);
        }

        // do we have to paginate?
        if (paginationInfo.getPaginate()) {
            paginationInfo.setTotalPages(((paginationInfo.getTotalResults() - 1) / paginationInfo
                    .getResultsPerPage()) + 1);

            // check if only one page
            // if (resultsPerPage >= totalResults) {
            if (paginationInfo.getTotalPages() == 1) {
                paginationInfo.setStartIndex(0);
                paginationInfo.setEndIndex(paginationInfo.getTotalResults() - 1);
                paginationInfo.setPageNumber(1);
                // design decision: should the resultsPerPage actually be set to totalResults?
                // since totalResults are already available preserve call value
            } else {
                // check if specified page number is on or beyoned last page (then use last page)
                if (paginationInfo.getPageNumber() >= paginationInfo.getTotalPages()) {
                    paginationInfo.setPageNumber(paginationInfo.getTotalPages());
                    paginationInfo.setStartIndex((paginationInfo.getTotalPages() - 1)
                        * paginationInfo.getResultsPerPage());
                    paginationInfo.setEndIndex(paginationInfo.getTotalResults() - 1);
                } else {
                    paginationInfo.setStartIndex((paginationInfo.getPageNumber() - 1)
                        * paginationInfo.getResultsPerPage());
                    paginationInfo.setEndIndex(paginationInfo.getStartIndex() + paginationInfo.getResultsPerPage()
                        - 1);
                }
            }
        } else {
            // default values assuming no pagination
            paginationInfo.setStartIndex(0);
            paginationInfo.setEndIndex(paginationInfo.getTotalResults() - 1);
            paginationInfo.setPageNumber(1);
            paginationInfo.setTotalPages(1);
            paginationInfo.setResultsPerPage(paginationInfo.getTotalResults());
        }

        return paginationInfo;
    }

    public static SortInfo calculateSorting(Form form) {
        SortInfo sortInfo = new SortInfo();

        String sortDirectionString = form.getFirstValue("sortdir");
        String sortFieldString = form.getFirstValue("sortby");

        // check for invalid input
        if ((sortDirectionString == null) || (sortFieldString == null)) {
            sortInfo.setSort(false);
            return sortInfo;
        }

        if ((sortDirectionString.isEmpty()) || (sortFieldString.isEmpty())) {
            sortInfo.setSort(false);
            return sortInfo;
        }

        sortInfo.setSort(true);

        // assume forward if get anything else but "reverse"
        if (sortDirectionString.toLowerCase().equals("reverse")) {
            sortInfo.setDirectionForward(false);
        } else {
            sortInfo.setDirectionForward(true);
        }

        // tough to type-check this one
        sortInfo.setSortField(sortFieldString);

        return sortInfo;
    }

    public static int compareIgnoreCaseNullSafe(String left, String right) {
        String leftString = left;
        String rightString = right;

        if (leftString == null) {
            leftString = EMPTY_STRING;
        }

        if (rightString == null) {
            rightString = EMPTY_STRING;
        }

        return leftString.compareToIgnoreCase(rightString);
    }

    // XML Response functions
    // ----------------------

    public static void setResponse(Response response, ResponseCode code) {
        setResponse(response, code, ResponseElements.NONE, null);
    }

    public static void setResponse(Response response, ResponseCode code, String additionalMessage) {
        List<String> elementValues = Arrays.asList(additionalMessage);

        setResponse(response, code, ResponseElements.ADDITIONAL_MESSAGE, elementValues);
    }

    public static void setResponse(Response response, ResponseCode code, int id) {
        List<String> elementValues = Arrays.asList(String.valueOf(id));

        setResponse(response, code, ResponseElements.ID, elementValues);
    }

    public static void setResponse(Response response, ResponseCode code, ResponseElements responseElements,
            List<String> elementValues) {
        try {
            DomRepresentation representation = new DomRepresentation(MediaType.TEXT_XML);
            Document document = representation.getDocument();

            // set response status
            setResponseStatus(response, code);
            setResponseElements(document, code, responseElements, elementValues);

            response.setEntity(new DomRepresentation(MediaType.TEXT_XML, document));

        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public static void setResponseError(Response response, ResponseCode code) {
        Representation representation = getResponseError(response, code);

        response.setEntity(representation);
    }

    public static void setResponseError(Response response, ResponseCode code, String additionalMessage) {
        Representation representation = getResponseError(response, code, additionalMessage);

        response.setEntity(representation);
    }

    public static void setResponseError(Response response, ResponseCode code, int id) {
        Representation representation = getResponseError(response, code, id);

        response.setEntity(representation);
    }

    public static void setResponseError(Response response, ResponseCode code, int id, String additionalMessage) {
        Representation representation = getResponseError(response, code, id, additionalMessage);

        response.setEntity(representation);
    }

    public static void setResponseError(Response response, ResponseCode code, String string,
            String additionalMessage) {
        Representation representation = getResponseError(response, code, string, additionalMessage);

        response.setEntity(representation);
    }

    public static void setResponseError(Response response, ResponseCode code, ResponseElements responseElements,
            List<String> elementValues) {
        Representation representation = getResponseError(response, code, responseElements, elementValues);

        response.setEntity(representation);
    }

    public static Representation getResponseError(Response response, ResponseCode code) {
        return getResponseError(response, code, ResponseElements.NONE, null);
    }

    public static Representation getResponseError(Response response, ResponseCode code, String additionalMessage) {
        List<String> elementValues = Arrays.asList(additionalMessage);

        return getResponseError(response, code, ResponseElements.ADDITIONAL_MESSAGE, elementValues);
    }

    public static Representation getResponseError(Response response, ResponseCode code, int id) {
        List<String> elementValues = Arrays.asList(String.valueOf(id));

        return getResponseError(response, code, ResponseElements.ID, elementValues);
    }

    public static Representation getResponseError(Response response, ResponseCode code, int id,
            String additionalMessage) {
        List<String> elementValues = Arrays.asList(String.valueOf(id), additionalMessage);

        return getResponseError(response, code, ResponseElements.ID_ADDITIONAL_MESSAGE, elementValues);
    }

    public static Representation getResponseError(Response response, ResponseCode code, String string,
            String additionalMessage) {
        List<String> elementValues = Arrays.asList(string, additionalMessage);

        return getResponseError(response, code, ResponseElements.STRING_ADDITIONAL_MESSAGE, elementValues);
    }

    public static Representation getResponseError(Response response, ResponseCode code,
            ResponseElements responseElements, List<String> elementValues) {
        try {
            DomRepresentation representation = new DomRepresentation(MediaType.TEXT_XML);
            Document document = representation.getDocument();

            setResponseStatus(response, code);
            setResponseElements(document, code, responseElements, elementValues);

            return representation;

        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        return null;
    }

    // number of elements specified by responseElements must equal number of values in
    // elementValues. elementNames and elementValues can be null for no data
    private static void setResponseElements(Document document, ResponseCode code,
            ResponseElements responseElements, List<String> elementValues) {
        List<String> elementNames = getElementNames(responseElements);

        // create root node
        Element elementResponse = document.createElement(ELEMENT_RESPONSE);
        document.appendChild(elementResponse);

        setResponseHeader(document, elementResponse, code, getResponseMessage(code));

        // add related data
        if ((elementNames != null) && (elementNames != null) && (!elementNames.isEmpty())
            && (!elementValues.isEmpty())) {
            Element elementData = document.createElement(ELEMENT_DATA);

            for (int index = 0; index < elementNames.size(); index++) {
                Element elementId = document.createElement(elementNames.get(index));
                elementId.appendChild(document.createTextNode(elementValues.get(index)));
                elementData.appendChild(elementId);
            }

            elementResponse.appendChild(elementData);
        }
    }

    private static void setResponseHeader(Document document, Element elementResponse, ResponseCode code,
            String message) {

        // add standard elements
        Element elementCode = document.createElement(ELEMENT_CODE);
        elementCode.appendChild(document.createTextNode(code.toString()));
        elementResponse.appendChild(elementCode);

        Element elementMessage = document.createElement(ELEMENT_MESSAGE);
        elementMessage.appendChild(document.createTextNode(message));
        elementResponse.appendChild(elementMessage);
    }

    public static String getResponseMessage(ResponseCode responseCode) {
        String responseMessage;

        switch (responseCode) {
        case ERROR_OBJECT_NOT_FOUND:
            responseMessage = "Could not retrieve object using ID";
            break;

        case ERROR_MISSING_ID:
            responseMessage = "ID value missing from request";
            break;

        case ERROR_READ_FAILED:
            responseMessage = "Read failed";
            break;

        case ERROR_UPDATE_FAILED:
            responseMessage = "Update failed";
            break;

        case ERROR_CREATE_FAILED:
            responseMessage = "Create failed";
            break;

        case ERROR_DELETE_FAILED:
            responseMessage = "Delete failed";
            break;

        case ERROR_ID_INVALID:
            responseMessage = "ID value is invalid";
            break;

        case ERROR_VALIDATION_FAILED:
            responseMessage = "Failed input validation";
            break;

        case ERROR_REFERENCE_EXISTS:
            responseMessage = "Reference to object still exists";
            break;

        case SUCCESS_UPDATED:
            responseMessage = "Updated";
            break;

        case SUCCESS_CREATED:
            responseMessage = "Created";
            break;

        case SUCCESS_DELETED:
            responseMessage = "Deleted";
            break;

        default:
            responseMessage = "Unhandled messageType";
            break;
        }

        return responseMessage;
    }

    private static void setResponseStatus(Response response, ResponseCode code) {
        // set response status based on code
        switch (code) {
        case SUCCESS_CREATED:
            response.setStatus(Status.SUCCESS_CREATED);
            break;

        case SUCCESS_DELETED:
            response.setStatus(Status.SUCCESS_OK);
            break;

        case SUCCESS_UPDATED:
            response.setStatus(Status.SUCCESS_OK);
            break;

        case ERROR_MISSING_ID:
            response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            break;

        case ERROR_OBJECT_NOT_FOUND:
            response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            break;

        case ERROR_UPDATE_FAILED:
            response.setStatus(Status.SERVER_ERROR_INTERNAL);
            break;

        case ERROR_CREATE_FAILED:
            response.setStatus(Status.SERVER_ERROR_INTERNAL);
            break;

        case ERROR_READ_FAILED:
            response.setStatus(Status.SERVER_ERROR_INTERNAL);
            break;

        case ERROR_DELETE_FAILED:
            response.setStatus(Status.SERVER_ERROR_INTERNAL);
            break;

        case ERROR_ID_INVALID:
            response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            break;

        case ERROR_VALIDATION_FAILED:
            response.setStatus(Status.CLIENT_ERROR_BAD_REQUEST);
            break;

        case ERROR_REFERENCE_EXISTS:
            response.setStatus(Status.SERVER_ERROR_INTERNAL);
            break;

        default:
            response.setStatus(Status.SUCCESS_OK);
        }
    }

    private static List<String> getElementNames(ResponseElements responseElements) {
        List<String> elementNames = new ArrayList<String>();

        switch (responseElements) {
        case ADDITIONAL_MESSAGE:
            elementNames.add(ELEMENT_ADDITIONAL_MESSAGE);
            break;

        case ID:
            elementNames.add(ELEMENT_ID);
            break;

        case ID_ADDITIONAL_MESSAGE:
            elementNames.add(ELEMENT_ID);
            elementNames.add(ELEMENT_ADDITIONAL_MESSAGE);
            break;

        case STRING_ADDITIONAL_MESSAGE:
            elementNames.add(ELEMENT_STRING);
            elementNames.add(ELEMENT_ADDITIONAL_MESSAGE);
            break;

        case NONE:
        default:
            break;

        }

        return elementNames;
    }

    // Data objects
    // ------------

    public static class PaginationInfo {
        private Boolean m_paginate = false;
        private int m_pageNumber;
        private int m_resultsPerPage;
        private int m_totalPages;
        private int m_totalResults;
        private int m_startIndex;
        private int m_endIndex;

        Boolean getPaginate() {
            return m_paginate;
        }

        void setPaginate(Boolean paginate) {
            this.m_paginate = paginate;
        }

        int getPageNumber() {
            return m_pageNumber;
        }

        void setPageNumber(int pageNumber) {
            this.m_pageNumber = pageNumber;
        }

        int getResultsPerPage() {
            return m_resultsPerPage;
        }

        void setResultsPerPage(int resultsPerPage) {
            this.m_resultsPerPage = resultsPerPage;
        }

        int getTotalPages() {
            return m_totalPages;
        }

        void setTotalPages(int totalPages) {
            this.m_totalPages = totalPages;
        }

        int getTotalResults() {
            return m_totalResults;
        }

        void setTotalResults(int totalResults) {
            this.m_totalResults = totalResults;
        }

        int getStartIndex() {
            return m_startIndex;
        }

        void setStartIndex(int startIndex) {
            this.m_startIndex = startIndex;
        }

        int getEndIndex() {
            return m_endIndex;
        }

        void setEndIndex(int endIndex) {
            this.m_endIndex = endIndex;
        }
    }

    public static class SortInfo {
        private Boolean m_sort = false;
        private Boolean m_directionForward = true;
        private String m_sortField = EMPTY_STRING;

        Boolean getSort() {
            return m_sort;
        }

        void setSort(Boolean sort) {
            this.m_sort = sort;
        }

        Boolean getDirectionForward() {
            return m_directionForward;
        }

        void setDirectionForward(Boolean directionForward) {
            this.m_directionForward = directionForward;
        }

        String getSortField() {
            return m_sortField;
        }

        void setSortField(String sortField) {
            this.m_sortField = sortField;
        }
    }

    public static class ValidationInfo {
        private Boolean m_valid = true;
        private String m_message = "Valid";
        private ResponseCode m_responseCode = ResponseCode.SUCCESS;

        public static enum StringConstraint {
            ONLY_LETTER_NUMBER_DASH_UNDERSCORE, ONLY_LETTER_NUMBER_DASH_UNDERSCORE_SYMBOL, ONLY_NUMBER,
            ONLY_NUMBER_ASTERISK, NOT_EMPTY
        }

        Boolean getValid() {
            return m_valid;
        }

        void setValid(Boolean valid) {
            this.m_valid = valid;
        }

        String getMessage() {
            return m_message;
        }

        void setMessage(String message) {
            this.m_message = message;
        }

        ResponseCode getResponseCode() {
            return m_responseCode;
        }

        void setResponseCode(ResponseCode responseCode) {
            this.m_responseCode = responseCode;
        }

        public void checkString(String string, String objectName, StringConstraint stringConstraint) {
            if (StringUtils.isEmpty(string)) {
                if (stringConstraint == StringConstraint.NOT_EMPTY) {
                    setError(objectName + ": Cannot be empty");
                }

                return;
            }

            switch (stringConstraint) {
            case ONLY_LETTER_NUMBER_DASH_UNDERSCORE:
                for (int i = 0; i < string.length(); i++) {
                    char ch = string.charAt(i);

                    if ((!Character.isLetterOrDigit(ch))
                        && (Character.getType(ch) != Character.CONNECTOR_PUNCTUATION) && (ch != '-')) {
                        setError(objectName + ": Must only contain letters, numbers, dashes, and underscores");
                    }
                }
                break;

            case ONLY_LETTER_NUMBER_DASH_UNDERSCORE_SYMBOL:
                for (int i = 0; i < string.length(); i++) {
                    char ch = string.charAt(i);

                    if (ch == ' ') {
                        setError(objectName
                            + ": Must only contain letters, numbers, dashes, underscores, and symbols");
                    }
                }
                break;

            case ONLY_NUMBER:
                for (int i = 0; i < string.length(); i++) {
                    char ch = string.charAt(i);

                    if ((!Character.isDigit(ch))) {
                        setError(objectName + ": Must only contain numbers");
                    }
                }
                break;

            case ONLY_NUMBER_ASTERISK:
                for (int i = 0; i < string.length(); i++) {
                    char ch = string.charAt(i);

                    if ((!Character.isDigit(ch)) && (ch != '*')) {
                        setError(objectName + ": Must only contain numbers and *");
                    }
                }

                break;

            default:
                break;
            }
        }

        public void checkRange(int value, String objectName, int lowerLimit, int upperLimit) {
            if ((value < lowerLimit) || (value > upperLimit)) {
                setError(objectName + ": must be between " + lowerLimit + " and " + upperLimit);
            }
        }

        public void checkStringRange(String string, String objectName, int lowerLimit, int upperLimit) {
            int intFromString;

            try {
                intFromString = Integer.parseInt(string);
            } catch (Exception ex) {
                setError(ex.getLocalizedMessage());
                return;
            }

            checkRange(intFromString, objectName, lowerLimit, upperLimit);
        }

        public void checkMember(String string, String objectName, List<String> stringList) {
            if (!(stringList.contains(string))) {
                String message = objectName + ": must be one of :";
                for (String item : stringList) {
                    message = message + item + " ";
                }
                setError(message);
            }

        }

        private void setError(String message) {
            this.setValid(false);
            this.setResponseCode(ERROR_VALIDATION_FAILED);
            this.setMessage(message);
        }
    }

    public static class ParameterInfo {
        private Boolean m_exists = false;
        private Boolean m_valid = false;

        Boolean getExists() {
            return m_exists;
        }

        void setExists(Boolean exists) {
            this.m_exists = exists;
        }

        Boolean getValid() {
            return m_valid;
        }

        void setValid(Boolean valid) {
            this.m_valid = valid;
        }
    }

    public static class IntParameterInfo extends ParameterInfo {
        private int m_value;
        private String m_valueString = EMPTY_STRING;

        int getValue() {
            // if invalid may not have proper value
            if (this.getValid()) {
                return m_value;
            } else {
                return -1;
            }
        }

        void setValue(int value) {
            this.m_value = value;
        }

        String getValueString() {
            return m_valueString;
        }

        void setValueString(String valueString) {
            this.m_valueString = valueString;
        }
    }

    public static class StringParameterInfo extends ParameterInfo {
        private String m_value = EMPTY_STRING;

        String getValue() {
            return m_value;
        }

        void setValue(String value) {
            this.m_value = value;
        }
    }

    public static class IntListParameterInfo extends ParameterInfo {
        private String m_listString;
        private List<Integer> m_intList;

        String getListString() {
            return m_listString;
        }

        void setListString(String listString) {
            this.m_listString = listString;
        }

        List<Integer> getIntList() {
            return m_intList;
        }

        void setIntList(List<Integer> intList) {
            this.m_intList = intList;
        }
    }

    public static class ConditionInfo {
        public static final String CONDITION_AVAILABLE_AGENTS = "available_agents";
        public static final String CONDITION_ELIGIBLE_AGENTS = "eligible_agents";
        public static final String CONDITION_CALLS_QUEUED = "calls_queued";
        public static final String CONDITION_QUEUE_POSITION = "queue_position";
        public static final String CONDITION_HOUR = "hour";
        public static final String CONDITION_WEEKDAY = "weekday";
        public static final String CONDITION_CLIENT_CALLS_QUEUED = "client_calls_queued";
        public static final String CONDITION_TICKS = "ticks";
        public static final String CONDITION_CLIENT = "client";
        public static final String CONDITION_MEDIA_TYPE = "media_type";
        public static final String CONDITION_CALLER_ID = "caller_id";
        public static final String CONDITION_CALLER_NAME = "caller_name";
        public static final String RELATION_IS = "is";
        public static final String RELATION_IS_NOT = "isNot";
        public static final String RELATION_GREATER = "greater";
        public static final String RELATION_LESS = "less";
        public static final String MEDIA_VOICE = "voice";
        public static final String MEDIA_EMAIL = "email";
        public static final String MEDIA_VOICEMAIL = "voicemail";
        public static final String MEDIA_CHAT = "chat";
        public static final String ACTION_ANNOUNCE = "announce";
        public static final String ACTION_SET_PRIORITY = "set_priority";
        public static final String VALUE_ACTION_VALUE = "Action Value";
        public static final String VALUE_RELATION = "Relation";
        public static final String VALUE_CONDITION = "Condition";
        public static final String VALUE_VALUE_CONDITION = "Value Condition";

        public static final List<String> CONDITION_IS = Arrays.asList(CONDITION_TICKS, CONDITION_CLIENT,
                CONDITION_MEDIA_TYPE, CONDITION_CALLER_ID, CONDITION_CALLER_NAME);
        public static final List<String> CONDITION_EQUALITY = Arrays.asList(CONDITION_AVAILABLE_AGENTS,
                CONDITION_ELIGIBLE_AGENTS, CONDITION_CALLS_QUEUED, CONDITION_QUEUE_POSITION, CONDITION_HOUR,
                CONDITION_WEEKDAY, CONDITION_CLIENT_CALLS_QUEUED);
        public static final List<String> IS_RELATION = Arrays.asList(RELATION_IS, RELATION_IS_NOT);
        public static final List<String> EQUALITY_RELATION = Arrays.asList(RELATION_IS, RELATION_GREATER,
                RELATION_LESS);
        public static final List<String> MEDIA_VALUES = Arrays.asList(MEDIA_VOICE, MEDIA_EMAIL, MEDIA_VOICEMAIL,
                MEDIA_CHAT);
    }

    // Common Rest Info objects
    // ------------------------

    static class PermissionRestInfoFull {
        private final String m_name;
        private final String m_label;
        private final String m_description;
        private final boolean m_defaultValue;
        private final Permission.Type m_type;
        private final boolean m_builtIn;

        public PermissionRestInfoFull(Permission permission) {
            m_name = permission.getName();
            m_label = permission.getLabel();
            m_description = permission.getDescription();
            m_defaultValue = permission.getDefaultValue();
            m_type = permission.getType();
            m_builtIn = permission.isBuiltIn();
        }

        public String getName() {
            return m_name;
        }

        public String getLabel() {
            return m_label;
        }

        public String getDescription() {
            return m_description;
        }

        public boolean getDefaultValue() {
            return m_defaultValue;
        }

        public Permission.Type getType() {
            return m_type;
        }

        public boolean getBuiltIn() {
            return m_builtIn;
        }
    }

    static class BranchRestInfo {
        private final int m_id;
        private final String m_name;
        private final String m_description;

        public BranchRestInfo(Branch branch) {
            m_id = branch.getId();
            m_name = branch.getName();
            m_description = branch.getDescription();
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
    }

    static class BranchRestInfoFull extends BranchRestInfo {
        private final Address m_address;
        private final String m_phoneNumber;
        private final String m_faxNumber;

        public BranchRestInfoFull(Branch branch) {
            super(branch);

            m_address = branch.getAddress();
            m_phoneNumber = branch.getPhoneNumber();
            m_faxNumber = branch.getFaxNumber();
        }

        public Address getAddress() {
            return m_address;
        }

        public String getPhoneNumber() {
            return m_phoneNumber;
        }

        public String getFaxNumber() {
            return m_faxNumber;
        }
    }

    static class UserGroupRestInfo {
        private final int m_id;
        private final String m_name;
        private final String m_description;

        public UserGroupRestInfo(Group userGroup) {
            m_id = userGroup.getId();
            m_name = userGroup.getName();
            m_description = userGroup.getDescription();
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
    }

    static class UserGroupRestInfoFull extends UserGroupRestInfo {
        private final BranchRestInfoFull m_branch;

        public UserGroupRestInfoFull(Group userGroup, BranchRestInfoFull branchRestInfo) {
            super(userGroup);

            m_branch = branchRestInfo;
        }

        public BranchRestInfoFull getBranch() {
            return m_branch;
        }
    }

    static class SettingPermissionRestInfo {
        private final String m_name;
        private final String m_label;
        private final String m_value;
        private final boolean m_defaultValue;

        public SettingPermissionRestInfo(String name, String label, String value, boolean defaultValue) {
            m_name = name;
            m_label = label;
            m_value = value;
            m_defaultValue = defaultValue;
        }

        public String getName() {
            return m_name;
        }

        public String getLabel() {
            return m_label;
        }

        public String getValue() {
            return m_value;
        }

        public boolean getDefaultValue() {
            return m_defaultValue;
        }
    }

    static class UserGroupPermissionRestInfoFull extends UserGroupRestInfo {
        private final List<SettingPermissionRestInfo> m_permissions;

        public UserGroupPermissionRestInfoFull(Group userGroup, List<SettingPermissionRestInfo> settingsRestInfo) {
            super(userGroup);

            m_permissions = settingsRestInfo;
        }

        public List<SettingPermissionRestInfo> getPermissions() {
            return m_permissions;
        }
    }

    static class UserRestInfo {
        private final int m_id;
        private final String m_lastName;
        private final String m_firstName;

        public UserRestInfo(User user) {
            m_id = user.getId();
            m_lastName = user.getLastName();
            m_firstName = user.getFirstName();
        }

        public int getId() {
            return m_id;
        }

        public String getLastName() {
            return m_lastName;
        }

        public String getFirstName() {
            return m_firstName;
        }
    }

    static class UserRestInfoFull extends UserRestInfo {
        private final String m_userName; // also called "User ID" in gui
        private final String m_pin;
        private final String m_sipPassword;
        private final String m_emailAddress;
        private final List<UserGroupRestInfo> m_groups;
        private final BranchRestInfo m_branch;
        private final List<AliasRestInfo> m_aliases;

        public UserRestInfoFull(User user, List<UserGroupRestInfo> userGroupsRestInfo,
                BranchRestInfo branchRestInfo, List<AliasRestInfo> aliasesRestInfo) {
            super(user);
            m_userName = user.getUserName();
            // pin is hardcoded to never display but must still be submitted
            m_pin = EMPTY_STRING;
            // sip password is hardcoded to never display but must still be submitted
            m_sipPassword = EMPTY_STRING;
            m_emailAddress = user.getEmailAddress();
            m_groups = userGroupsRestInfo;
            m_branch = branchRestInfo;
            m_aliases = aliasesRestInfo;
        }

        public String getUserName() {
            return m_userName;
        }

        public String getPin() {
            return m_pin;
        }

        public String getSipPassword() {
            return m_sipPassword;
        }

        public String getEmailAddress() {
            return m_emailAddress;
        }

        public List<UserGroupRestInfo> getGroups() {
            return m_groups;
        }

        public BranchRestInfo getBranch() {
            return m_branch;
        }

        public List<AliasRestInfo> getAliases() {
            return m_aliases;
        }
    }

    static class UserPermissionRestInfoFull extends UserRestInfo {
        private final List<SettingPermissionRestInfo> m_permissions;

        public UserPermissionRestInfoFull(User user, List<SettingPermissionRestInfo> settingsRestInfo) {
            super(user);

            m_permissions = settingsRestInfo;
        }

        public List<SettingPermissionRestInfo> getPermissions() {
            return m_permissions;
        }
    }

    static class AliasRestInfo {

        private final String m_alias;

        public AliasRestInfo(String alias) {
            m_alias = alias;
        }

        public String getAlias() {
            return m_alias;
        }
    }

    // Common OpenACD Rest Info objects
    // ------------------------

    static class MetadataRestInfo {
        private final int m_totalResults;
        private final int m_currentPage;
        private final int m_totalPages;
        private final int m_resultsPerPage;

        public MetadataRestInfo(PaginationInfo paginationInfo) {
            m_totalResults = paginationInfo.getTotalResults();
            m_currentPage = paginationInfo.getPageNumber();
            m_totalPages = paginationInfo.getTotalPages();
            m_resultsPerPage = paginationInfo.getResultsPerPage();
        }

        public int getTotalResults() {
            return m_totalResults;
        }

        public int getCurrentPage() {
            return m_currentPage;
        }

        public int getTotalPages() {
            return m_totalPages;
        }

        public int getResultsPerPage() {
            return m_resultsPerPage;
        }
    }
}
