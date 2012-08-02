/*
 *
 *  Copyright (C) 2012 PATLive, D. Waseem, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdReleaseCodesResource.java - A Restlet to read Release Code data from OpenACD within SipXecs
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
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
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
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdReleaseCode;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdReleaseCodeRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class OpenAcdReleaseCodesResource extends Resource {

    private static final String ELEMENT_NAME_RELEASECODEBUNDLE = "openacd-release-code";
    private static final String ELEMENT_NAME_RELEASECODE = "release-code";

    private OpenAcdContext m_openAcdContext;
    private Form m_form;

    // use to define all possible sort fields
    enum SortField {
        LABEL, DESCRIPTION, NONE;

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

    // GET - Retrieve all and single Release Code
    // -------------------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdReleaseCode releaseCode;
        OpenAcdReleaseCodeRestInfo releaseCodeRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                releaseCode = m_openAcdContext.getReleaseCodeById(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                releaseCodeRestInfo = createReleaseCodeRestInfo(releaseCode);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new OpenAcdReleaseCodeRepresentation(variant.getMediaType(), releaseCodeRestInfo);
        }

        // if not single, process request for list
        List<OpenAcdReleaseCode> releaseCodes = m_openAcdContext.getReleaseCodes();
        List<OpenAcdReleaseCodeRestInfo> releaseCodesRestInfo = new ArrayList<OpenAcdReleaseCodeRestInfo>();
        MetadataRestInfo metadataRestInfo;

        // sort groups if specified
        sortReleaseCodes(releaseCodes);

        // set requested records and get resulting metadata
        metadataRestInfo = addReleaseCodes(releaseCodesRestInfo, releaseCodes);

        // create final restinfo
        OpenAcdReleaseCodesBundleRestInfo releaseCodesBundleRestInfo =
                new OpenAcdReleaseCodesBundleRestInfo(releaseCodesRestInfo, metadataRestInfo);

        return new OpenAcdReleaseCodesRepresentation(variant.getMediaType(), releaseCodesBundleRestInfo);
    }

    // PUT - Update or Add single Skill
    // --------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get from request body
        OpenAcdReleaseCodeRepresentation representation = new OpenAcdReleaseCodeRepresentation(entity);
        OpenAcdReleaseCodeRestInfo releaseCodeRestInfo = representation.getObject();
        OpenAcdReleaseCode releaseCode;

        // validate input for update or create
        ValidationInfo validationInfo = validate(releaseCodeRestInfo);

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
                releaseCode = m_openAcdContext.getReleaseCodeById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateReleaseCode(releaseCode, releaseCodeRestInfo);
                m_openAcdContext.saveReleaseCode(releaseCode);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, releaseCode.getId());
            return;
        }

        // if not single, add new item
        try {
            releaseCode = createReleaseCode(releaseCodeRestInfo);
            m_openAcdContext.saveReleaseCode(releaseCode);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, releaseCode.getId());
    }

    // DELETE - Delete single Skill
    // ----------------------------

    // deleteReleaseCode() not available from openAcdContext
    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        @SuppressWarnings("unused")
        OpenAcdReleaseCode releaseCode;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            // do not need object to delete, but confirm existence for error message
            try {
                releaseCode = m_openAcdContext.getReleaseCodeById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            try {
                // release codes deleted using collection of ids, not releaseCode object
                Collection<Integer> releaseCodeIds = new HashSet<Integer>();
                releaseCodeIds.add(parameterInfo.getValue());
                m_openAcdContext.removeReleaseCodes(releaseCodeIds);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, parameterInfo.getValue());
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
    private ValidationInfo validate(OpenAcdReleaseCodeRestInfo restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String label = restInfo.getLabel();
        // release code object will allow store of bias other value than -1, 0, or 1,
        // but then current SipXconfig administrative UI will display nothing for bias name.
        int bias = restInfo.getBias();

        validationInfo.checkString(label, "Label", StringConstraint.NOT_EMPTY);
        validationInfo.checkRange(bias, "Bias", -1, 1);

        return validationInfo;
    }

    private OpenAcdReleaseCodeRestInfo createReleaseCodeRestInfo(OpenAcdReleaseCode releaseCode) {
        OpenAcdReleaseCodeRestInfo releaseCodeRestInfo;

        releaseCodeRestInfo = new OpenAcdReleaseCodeRestInfo(releaseCode);

        return releaseCodeRestInfo;
    }

    private MetadataRestInfo addReleaseCodes(List<OpenAcdReleaseCodeRestInfo> releaseCodesRestInfo,
            List<OpenAcdReleaseCode> releaseCodes) {
        OpenAcdReleaseCode releaseCode;
        OpenAcdReleaseCodeRestInfo releaseCodeRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, releaseCodes.size());

        // create list of releaseCode restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            releaseCode = releaseCodes.get(index);
            releaseCodeRestInfo = createReleaseCodeRestInfo(releaseCode);
            releaseCodesRestInfo.add(releaseCodeRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortReleaseCodes(List<OpenAcdReleaseCode> releaseCodes) {
        // sort groups if requested
        SortInfo sortInfo = RestUtilities.calculateSorting(m_form);

        if (!sortInfo.getSort()) {
            return;
        }

        SortField sortField = SortField.toSortField(sortInfo.getSortField());
        boolean sortForward = sortInfo.getDirectionForward();

        switch (sortField) {
        case LABEL:
            if (sortForward) {
                Collections.sort(releaseCodes, new LabelComparator());
            } else {
                Collections.sort(releaseCodes, new ReverseComparator(new LabelComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(releaseCodes, new DescriptionComparator());
            } else {
                Collections.sort(releaseCodes, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateReleaseCode(OpenAcdReleaseCode releaseCode, OpenAcdReleaseCodeRestInfo releaseCodeRestInfo)
        throws ResourceException {
        releaseCode.setLabel(releaseCodeRestInfo.getLabel());
        releaseCode.setDescription(releaseCodeRestInfo.getDescription());
        releaseCode.setBias(releaseCodeRestInfo.getBias());
    }

    private OpenAcdReleaseCode createReleaseCode(OpenAcdReleaseCodeRestInfo releaseCodeRestInfo)
        throws ResourceException {
        OpenAcdReleaseCode releaseCode = new OpenAcdReleaseCode();

        updateReleaseCode(releaseCode, releaseCodeRestInfo);

        return releaseCode;
    }

    // Sorting comparator classes
    // --------------------------

    private static class LabelComparator implements Comparator<OpenAcdReleaseCode> {
        @Override
        public int compare(OpenAcdReleaseCode item1, OpenAcdReleaseCode item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getLabel(), item2.getLabel());
        }
    }

    private static class DescriptionComparator implements Comparator<OpenAcdReleaseCode> {
        @Override
        public int compare(OpenAcdReleaseCode item1, OpenAcdReleaseCode item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    // REST Representations
    // --------------------

    static class OpenAcdReleaseCodesRepresentation extends XStreamRepresentation<OpenAcdReleaseCodesBundleRestInfo> {

        public OpenAcdReleaseCodesRepresentation(MediaType mediaType, OpenAcdReleaseCodesBundleRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdReleaseCodesRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_RELEASECODEBUNDLE, OpenAcdReleaseCodesBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_RELEASECODE, OpenAcdReleaseCodeRestInfo.class);
        }
    }

    static class OpenAcdReleaseCodeRepresentation extends XStreamRepresentation<OpenAcdReleaseCodeRestInfo> {

        public OpenAcdReleaseCodeRepresentation(MediaType mediaType, OpenAcdReleaseCodeRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdReleaseCodeRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_RELEASECODE, OpenAcdReleaseCodeRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdReleaseCodesBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<OpenAcdReleaseCodeRestInfo> m_releaseCodes;

        public OpenAcdReleaseCodesBundleRestInfo(List<OpenAcdReleaseCodeRestInfo> releaseCodes,
                MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_releaseCodes = releaseCodes;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<OpenAcdReleaseCodeRestInfo> getReleaseCodes() {
            return m_releaseCodes;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

}
