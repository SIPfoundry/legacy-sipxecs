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
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkill;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkillGroup;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.OpenAcdSkillRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class OpenAcdSkillsResource extends Resource {

    private static final String ELEMENT_NAME_SKILLBUNDLE = "openacd-skill";
    private static final String ELEMENT_NAME_SKILL = "skill";

    private OpenAcdContext m_openAcdContext;
    private Form m_form;

    // use to define all possible sort fields
    private enum SortField {
        NAME, DESCRIPTION, ATOM, GROUPNAME, NONE;

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
        OpenAcdSkill skill;
        OpenAcdSkillRestInfoFull skillRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                skill = m_openAcdContext.getSkillById(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                skillRestInfo = createSkillRestInfo(skill);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new OpenAcdSkillRepresentation(variant.getMediaType(), skillRestInfo);
        }

        // if not single, process request for list
        List<OpenAcdSkill> skills = m_openAcdContext.getSkills();
        List<OpenAcdSkillRestInfoFull> skillsRestInfo = new ArrayList<OpenAcdSkillRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort groups if specified
        sortSkills(skills);

        // set requested agents groups and get resulting metadata
        metadataRestInfo = addSkills(skillsRestInfo, skills);

        // create final restinfo
        OpenAcdSkillsBundleRestInfo skillsBundleRestInfo =
                new OpenAcdSkillsBundleRestInfo(skillsRestInfo, metadataRestInfo);

        return new OpenAcdSkillsRepresentation(variant.getMediaType(), skillsBundleRestInfo);
    }

    // PUT - Update or Add single Skill
    // --------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get from request body
        OpenAcdSkillRepresentation representation = new OpenAcdSkillRepresentation(entity);
        OpenAcdSkillRestInfoFull skillRestInfo = representation.getObject();
        OpenAcdSkill skill;

        // validate input for update or create
        ValidationInfo validationInfo = validate(skillRestInfo);

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
                skill = m_openAcdContext.getSkillById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateSkill(skill, skillRestInfo);
                m_openAcdContext.saveSkill(skill);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, skill.getId());
            return;
        }

        // if not single, add new item
        try {
            skill = createSkill(skillRestInfo);
            m_openAcdContext.saveSkill(skill);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, skill.getId());
    }

    // DELETE - Delete single Skill
    // ----------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        OpenAcdSkill skill;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                skill = m_openAcdContext.getSkillById(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            try {
                m_openAcdContext.deleteSkill(skill);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, skill.getId());
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
    private ValidationInfo validate(OpenAcdSkillRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String nameObject = "Name";
        String atomObject = "Atom";

        String name = restInfo.getName();
        String atom = restInfo.getAtom();

        validationInfo.checkString(name, nameObject, StringConstraint.NOT_EMPTY);
        validationInfo.checkString(name, nameObject, StringConstraint.ONLY_LETTER_NUMBER_DASH_UNDERSCORE);
        validationInfo.checkString(atom, atomObject, StringConstraint.NOT_EMPTY);
        validationInfo.checkString(atom, atomObject, StringConstraint.ONLY_LETTER_NUMBER_DASH_UNDERSCORE);

        return validationInfo;
    }

    private OpenAcdSkillRestInfoFull createSkillRestInfo(OpenAcdSkill skill) {
        OpenAcdSkillRestInfoFull skillRestInfo = null;

        skillRestInfo = new OpenAcdSkillRestInfoFull(skill);

        return skillRestInfo;
    }

    private MetadataRestInfo addSkills(List<OpenAcdSkillRestInfoFull> skillsRestInfo, List<OpenAcdSkill> skills) {
        OpenAcdSkill skill;
        OpenAcdSkillRestInfoFull skillRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, skills.size());

        // create list of skill restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            skill = skills.get(index);
            skillRestInfo = createSkillRestInfo(skill);
            skillsRestInfo.add(skillRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortSkills(List<OpenAcdSkill> skills) {
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
                Collections.sort(skills, new NameComparator());
            } else {
                Collections.sort(skills, new ReverseComparator(new NameComparator()));
            }
            break;

        case GROUPNAME:
            if (sortForward) {
                Collections.sort(skills, new GroupNameComparator());
            } else {
                Collections.sort(skills, new ReverseComparator(new GroupNameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(skills, new DescriptionComparator());
            } else {
                Collections.sort(skills, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        case ATOM:
            if (sortForward) {
                Collections.sort(skills, new AtomComparator());
            } else {
                Collections.sort(skills, new ReverseComparator(new AtomComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateSkill(OpenAcdSkill skill, OpenAcdSkillRestInfoFull skillRestInfo) {
        OpenAcdSkillGroup skillGroup;

        skill.setName(skillRestInfo.getName());
        skill.setDescription(skillRestInfo.getDescription());

        skillGroup = getSkillGroup(skillRestInfo);
        skill.setGroup(skillGroup);
    }

    private OpenAcdSkill createSkill(OpenAcdSkillRestInfoFull skillRestInfo) throws ResourceException {
        OpenAcdSkill skill = new OpenAcdSkill();

        // only set atom when creating (cannot update)
        skill.setAtom(skillRestInfo.getAtom());

        updateSkill(skill, skillRestInfo);

        return skill;
    }

    private OpenAcdSkillGroup getSkillGroup(OpenAcdSkillRestInfoFull skillRestInfo) {
        OpenAcdSkillGroup skillGroup;
        int groupId = skillRestInfo.getGroupId();
        skillGroup = m_openAcdContext.getSkillGroupById(groupId);

        return skillGroup;
    }

    // Sorting comparator classes
    // --------------------------

    private static class NameComparator implements Comparator<OpenAcdSkill> {
        @Override
        public int compare(OpenAcdSkill item1, OpenAcdSkill item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class GroupNameComparator implements Comparator<OpenAcdSkill> {
        @Override
        public int compare(OpenAcdSkill item1, OpenAcdSkill item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getGroupName(), item2.getGroupName());
        }
    }

    private static class DescriptionComparator implements Comparator<OpenAcdSkill> {
        @Override
        public int compare(OpenAcdSkill item1, OpenAcdSkill item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    private static class AtomComparator implements Comparator<OpenAcdSkill> {
        @Override
        public int compare(OpenAcdSkill item1, OpenAcdSkill item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getAtom(), item2.getAtom());
        }
    }

    // REST Representations
    // --------------------

    static class OpenAcdSkillsRepresentation extends XStreamRepresentation<OpenAcdSkillsBundleRestInfo> {

        public OpenAcdSkillsRepresentation(MediaType mediaType, OpenAcdSkillsBundleRestInfo object) {
            super(mediaType, object);
        }

        public OpenAcdSkillsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_SKILLBUNDLE, OpenAcdSkillsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_SKILL, OpenAcdSkillRestInfoFull.class);
        }
    }

    static class OpenAcdSkillRepresentation extends XStreamRepresentation<OpenAcdSkillRestInfoFull> {

        public OpenAcdSkillRepresentation(MediaType mediaType, OpenAcdSkillRestInfoFull object) {
            super(mediaType, object);
        }

        public OpenAcdSkillRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_SKILL, OpenAcdSkillRestInfoFull.class);
        }
    }

    // REST info objects
    // -----------------

    static class OpenAcdSkillsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<OpenAcdSkillRestInfoFull> m_skills;

        public OpenAcdSkillsBundleRestInfo(List<OpenAcdSkillRestInfoFull> skills, MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_skills = skills;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<OpenAcdSkillRestInfoFull> getSkills() {
            return m_skills;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setOpenAcdContext(OpenAcdContext openAcdContext) {
        m_openAcdContext = openAcdContext;
    }

}
