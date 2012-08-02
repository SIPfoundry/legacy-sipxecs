/*
 *
 *  Copyright (C) 2012 PATLive, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  UserGroupsResource.java - A Restlet to read User Group data from SipXecs
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
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.branch.BranchManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.rest.RestUtilities.BranchRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.UserGroupRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class UserGroupsResource extends Resource {

    private static final String ELEMENT_NAME_USERGROUPBUNDLE = "user-group";
    private static final String ELEMENT_NAME_USERGROUP = "group";

    private CoreContext m_coreContext;
    private SettingDao m_settingContext; // saveGroup is not available through corecontext
    private BranchManager m_branchManager;
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

    @Override
    public boolean allowDelete() {
        return true;
    }

    // GET - Retrieve all and single Skill
    // -----------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        Group userGroup;
        UserGroupRestInfoFull userGroupRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                userGroup = m_settingContext.getGroup(parameterInfo.getValue());
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                        .getValue());
            }

            try {
                userGroupRestInfo = createUserGroupRestInfo(userGroup);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new UserGroupRepresentation(variant.getMediaType(), userGroupRestInfo);
        }

        // if not single, process request for list
        List<Group> userGroups = m_coreContext.getGroups();
        List<UserGroupRestInfoFull> userGroupsRestInfo = new ArrayList<UserGroupRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort if specified
        sortUserGroups(userGroups);

        // set requested items and get resulting metadata
        metadataRestInfo = addUserGroups(userGroupsRestInfo, userGroups);

        // create final restinfo
        UserGroupsBundleRestInfo userGroupsBundleRestInfo =
                new UserGroupsBundleRestInfo(userGroupsRestInfo, metadataRestInfo);

        return new UserGroupsRepresentation(variant.getMediaType(), userGroupsBundleRestInfo);
    }

    // PUT - Update or Add single Skill
    // --------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get from request body
        UserGroupRepresentation representation = new UserGroupRepresentation(entity);
        UserGroupRestInfoFull userGroupRestInfo = representation.getObject();
        Group userGroup;

        // validate input for update or create
        ValidationInfo validationInfo = validate(userGroupRestInfo);

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
                userGroup = m_settingContext.getGroup(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                updateUserGroup(userGroup, userGroupRestInfo);
                m_settingContext.saveGroup(userGroup);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, userGroup.getId());
            return;
        }

        // if not single, add new item
        try {
            userGroup = createUserGroup(userGroupRestInfo);
            m_settingContext.saveGroup(userGroup);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, userGroup.getId());
    }

    // DELETE - Delete single Skill
    // ----------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        @SuppressWarnings("unused")
        Group userGroup;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            // do not need object to delete, but confirm existence for error message
            try {
                userGroup = m_settingContext.getGroup(parameterInfo.getValue());
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                return;
            }

            try {
                // user groups deleted using collection of ids, not user group object
                List<Integer> userGroupIds = new ArrayList<Integer>();
                userGroupIds.add(parameterInfo.getValue());
                m_settingContext.deleteGroups(userGroupIds);
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
    private ValidationInfo validate(UserGroupRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String name = restInfo.getName();

        validationInfo.checkString(name, "Name", StringConstraint.NOT_EMPTY);

        return validationInfo;
    }

    private UserGroupRestInfoFull createUserGroupRestInfo(Group group) {
        UserGroupRestInfoFull userGroupRestInfo = null;
        BranchRestInfoFull branchRestInfo = null;
        Branch branch = null;

        // group may not have branch assigned
        branch = group.getBranch();
        if (branch != null) {
            branchRestInfo = createBranchRestInfo(branch.getId());
        }

        userGroupRestInfo = new UserGroupRestInfoFull(group, branchRestInfo);

        return userGroupRestInfo;
    }

    private BranchRestInfoFull createBranchRestInfo(int id) {
        BranchRestInfoFull branchRestInfo = null;

        Branch branch = m_branchManager.getBranch(id);
        branchRestInfo = new BranchRestInfoFull(branch);

        return branchRestInfo;
    }

    private MetadataRestInfo addUserGroups(List<UserGroupRestInfoFull> userGroupsRestInfo, List<Group> userGroups) {
        UserGroupRestInfoFull userGroupRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, userGroups.size());

        // create list of restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            Group userGroup = userGroups.get(index);

            userGroupRestInfo = createUserGroupRestInfo(userGroup);
            userGroupsRestInfo.add(userGroupRestInfo);
        }

        // create metadata about restinfos
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortUserGroups(List<Group> userGroups) {
        // sort if requested
        SortInfo sortInfo = RestUtilities.calculateSorting(m_form);

        if (!sortInfo.getSort()) {
            return;
        }

        SortField sortField = SortField.toSortField(sortInfo.getSortField());
        boolean sortForward = sortInfo.getDirectionForward();

        switch (sortField) {
        case NAME:
            if (sortForward) {
                Collections.sort(userGroups, new NameComparator());
            } else {
                Collections.sort(userGroups, new ReverseComparator(new NameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(userGroups, new DescriptionComparator());
            } else {
                Collections.sort(userGroups, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateUserGroup(Group userGroup, UserGroupRestInfoFull userGroupRestInfo) {
        Branch branch;

        userGroup.setName(userGroupRestInfo.getName());
        userGroup.setDescription(userGroupRestInfo.getDescription());

        branch = getBranch(userGroupRestInfo);
        userGroup.setBranch(branch);
    }

    private Group createUserGroup(UserGroupRestInfoFull userGroupRestInfo) {
        Group userGroup = new Group();

        // apparently there is a special Resource value for user groups
        userGroup.setResource(CoreContext.USER_GROUP_RESOURCE_ID);

        updateUserGroup(userGroup, userGroupRestInfo);

        return userGroup;
    }

    private Branch getBranch(UserGroupRestInfoFull userGroupRestInfo) {
        Branch branch = null;
        BranchRestInfoFull branchRestInfo = userGroupRestInfo.getBranch();

        if (branchRestInfo != null) {
            branch = m_branchManager.getBranch(branchRestInfo.getId());
        }

        return branch;
    }

    // Sorting comparator classes
    // --------------------------

    private static class NameComparator implements Comparator<Group> {
        @Override
        public int compare(Group item1, Group item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class DescriptionComparator implements Comparator<Group> {
        @Override
        public int compare(Group item1, Group item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    // REST Representations
    // --------------------

    static class UserGroupsRepresentation extends XStreamRepresentation<UserGroupsBundleRestInfo> {

        public UserGroupsRepresentation(MediaType mediaType, UserGroupsBundleRestInfo object) {
            super(mediaType, object);
        }

        public UserGroupsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_USERGROUPBUNDLE, UserGroupsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_USERGROUP, UserGroupRestInfoFull.class);
        }
    }

    static class UserGroupRepresentation extends XStreamRepresentation<UserGroupRestInfoFull> {

        public UserGroupRepresentation(MediaType mediaType, UserGroupRestInfoFull object) {
            super(mediaType, object);
        }

        public UserGroupRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_USERGROUP, UserGroupRestInfoFull.class);
        }
    }

    // REST info objects
    // -----------------

    static class UserGroupsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<UserGroupRestInfoFull> m_groups;

        public UserGroupsBundleRestInfo(List<UserGroupRestInfoFull> userGroups, MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_groups = userGroups;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<UserGroupRestInfoFull> getGroups() {
            return m_groups;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setSettingDao(SettingDao settingContext) {
        m_settingContext = settingContext;
    }

    @Required
    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }

}
