/*
 *
 *  Copyright (C) 2012 PATLive, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  OpenAcdAgentGroupsResource.java - A Restlet to read Skill data from OpenACD within SipXecs
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
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_REFERENCE_EXISTS;
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
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkill;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkillGroup;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdSkillGroupRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class OpenAcdSkillGroupsResource extends Resource {

    private static final String ELEMENT_NAME_SKILLGROUPBUNDLE = "openacd-skill-group";
    private static final String ELEMENT_NAME_SKILLGROUP = "group";

    private OpenAcdContext m_openAcdContext;
    private Form m_form;

    // use to define all possible sort fields
    private enum SortField {
        NAME, DESCRIPTION, NUMBERSKILLS, NONE;

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

    // GET - Retrieve all and single Skill
    // -----------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdSkillGroup skillGroup;
        OpenAcdSkillGroupRestInfo skillGroupRestInfo;

        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                skillGroup = m_openAcdContext.getSkillGroupById(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                skillGroupRestInfo = createSkillGroupRestInfo(skillGroup);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new OpenAcdSkillGroupRepresentation(variant.getMediaType(), skillGroupRestInfo);
        }

        // if not single, process request for list
        List<OpenAcdSkillGroup> skillGroups = m_openAcdContext.getSkillGroups();
        List<OpenAcdSkillGroupRestInfo> skillGroupsRestInfo = new ArrayList<OpenAcdSkillGroupRestInfo>();
        MetadataRestInfo metadataRestInfo;

        // sort groups if specified
        sortSkillGroups(skillGroups);

        // set requested records and get resulting metadata
        metadataRestInfo = addSkillGroups(skillGroupsRestInfo, skillGroups);

        // create final restinfo
        OpenAcdSkillGroupsBundleRestInfo skillGroupsBundleRestInfo =
                new OpenAcdSkillGroupsBundleRestInfo(skillGroupsRestInfo, metadataRestInfo);

        return new OpenAcdSkillGroupsRepresentation(variant.getMediaType(), skillGroupsBundleRestInfo);
    }

    // PUT - Update or Create single
    // -----------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get from request body
        OpenAcdSkillGroupRepresentation representation = new OpenAcdSkillGroupRepresentation(entity);
        OpenAcdSkillGroupRestInfo skillGroupRestInfo = representation.getObject();
        OpenAcdSkillGroup skillGroup;

        // validate input for update or create
        ValidationInfo validationInfo = validate(skillGroupRestInfo);

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
                skillGroup = m_openAcdContext.getSkillGroupById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateSkillGroup(skillGroup, skillGroupRestInfo);
                m_openAcdContext.saveSkillGroup(skillGroup);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, skillGroup.getId());
            return;
        }

        // if not single, add new item
        try {
            skillGroup = createSkillGroup(skillGroupRestInfo);
            m_openAcdContext.saveSkillGroup(skillGroup);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, skillGroup.getId());
    }

    // DELETE - Delete single
    // ----------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        List<OpenAcdSkill> skills;
        @SuppressWarnings("unused")
        OpenAcdSkillGroup skillGroup;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            // do not need object to delete, but confirm existence for error message
            try {
                skillGroup = m_openAcdContext.getSkillGroupById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // sipxconfig ui does not allow delete of group with existing skills
            skills = m_openAcdContext.getSkills();
            for (OpenAcdSkill skill : skills) {
                if (skill.getGroup().getId() == parameterInfo.getValue()) {
                    RestUtilities.setResponseError(getResponse(), ERROR_REFERENCE_EXISTS, skill.getName());
                    return;
                }
            }

            try {
                // skill groups deleted using collection of ids, not a skill group object
                Collection<Integer> skillGroupIds = new HashSet<Integer>();
                skillGroupIds.add(parameterInfo.getValue());
                m_openAcdContext.removeSkillGroups(skillGroupIds);
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
    private ValidationInfo validate(OpenAcdSkillGroupRestInfo restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String nameObject = "Name";

        String name = restInfo.getName();

        validationInfo.checkString(name, nameObject, StringConstraint.NOT_EMPTY);
        validationInfo.checkString(name, nameObject, StringConstraint.ONLY_LETTER_NUMBER_DASH_UNDERSCORE);

        return validationInfo;
    }

    private OpenAcdSkillGroupRestInfo createSkillGroupRestInfo(OpenAcdSkillGroup skillGroup) {
        OpenAcdSkillGroupRestInfo skillGroupRestInfo;

        skillGroupRestInfo = new OpenAcdSkillGroupRestInfo(skillGroup);

        return skillGroupRestInfo;
    }

    private MetadataRestInfo addSkillGroups(List<OpenAcdSkillGroupRestInfo> skillsRestInfo,
            List<OpenAcdSkillGroup> skillGroups) {
        OpenAcdSkillGroup skillGroup;
        OpenAcdSkillGroupRestInfo skillRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, skillGroups.size());

        // create list of skill restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            skillGroup = skillGroups.get(index);
            skillRestInfo = createSkillGroupRestInfo(skillGroup);
            skillsRestInfo.add(skillRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortSkillGroups(List<OpenAcdSkillGroup> skillGroups) {
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
                Collections.sort(skillGroups, new NameComparator());
            } else {
                Collections.sort(skillGroups, new ReverseComparator(new NameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(skillGroups, new DescriptionComparator());
            } else {
                Collections.sort(skillGroups, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        case NUMBERSKILLS:
            if (sortForward) {
                Collections.sort(skillGroups, new NumberSkillsComparator());
            } else {
                Collections.sort(skillGroups, new ReverseComparator(new NumberSkillsComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateSkillGroup(OpenAcdSkillGroup skillGroup, OpenAcdSkillGroupRestInfo skillGroupRestInfo)
        throws ResourceException {
        skillGroup.setName(skillGroupRestInfo.getName());
        skillGroup.setDescription(skillGroupRestInfo.getDescription());
    }

    private OpenAcdSkillGroup createSkillGroup(OpenAcdSkillGroupRestInfo skillGroupRestInfo)
        throws ResourceException {
        OpenAcdSkillGroup skillGroup = new OpenAcdSkillGroup();

        updateSkillGroup(skillGroup, skillGroupRestInfo);

        return skillGroup;
    }

    // Sorting comparator classes
    // --------------------------

    private static class NameComparator implements Comparator<OpenAcdSkillGroup> {
        @Override
        public int compare(OpenAcdSkillGroup item1, OpenAcdSkillGroup item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class DescriptionComparator implements Comparator<OpenAcdSkillGroup> {
        @Override
        public int compare(OpenAcdSkillGroup item1, OpenAcdSkillGroup item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    private static class NumberSkillsComparator implements Comparator<OpenAcdSkillGroup> {
        @Override
        public int compare(OpenAcdSkillGroup item1, OpenAcdSkillGroup item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(String.valueOf(item1.getSkills().size()), String
                    .valueOf(item2.getSkills().size()));
        }
    }

    // REST Representations
    // --------------------

    static class OpenAcdSkillGroupsRepresentation extends XStreamRepresentation<OpenAcdSkillGroupsBundleRestInfo> {

        public OpenAcdSkillGroupsRepresentation(MediaType mediaType, OpenAcdSkillGroupsBundleRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdSkillGroupsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_SKILLGROUPBUNDLE, OpenAcdSkillGroupsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_SKILLGROUP, OpenAcdSkillGroupRestInfo.class);
        }
    }

    static class OpenAcdSkillGroupRepresentation extends XStreamRepresentation<OpenAcdSkillGroupRestInfo> {

        public OpenAcdSkillGroupRepresentation(MediaType mediaType, OpenAcdSkillGroupRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdSkillGroupRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_SKILLGROUP, OpenAcdSkillGroupRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdSkillGroupsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<OpenAcdSkillGroupRestInfo> m_groups;

        public OpenAcdSkillGroupsBundleRestInfo(List<OpenAcdSkillGroupRestInfo> skillGroups,
                MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_groups = skillGroups;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<OpenAcdSkillGroupRestInfo> getGroups() {
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
