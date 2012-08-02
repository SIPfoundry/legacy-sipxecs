/*
 *
 *  Copyright (C) 2012 PATLive, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  UsersResource.java - A Restlet to read User data from SipXecs
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
import static org.sipfoundry.sipxconfig.rest.RestUtilities.REQUEST_ATTRIBUTE_BRANCH;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.REQUEST_ATTRIBUTE_ID;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.REQUEST_ATTRIBUTE_IDLIST;
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
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

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
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.rest.RestUtilities.AliasRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.BranchRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.BranchRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntListParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.UserGroupRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.UserRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class UsersResource extends Resource {
    private static final String USER = "user";

    private static final String ELEMENT_NAME_USERBUNDLE = USER;
    private static final String ELEMENT_NAME_USER = USER;
    private static final String ELEMENT_NAME_USERGROUP = "group";
    private static final String ELEMENT_NAME_BRANCH = "branch";
    private static final String ELEMENT_NAME_ALIAS = "alias";

    private CoreContext m_coreContext;
    private BranchManager m_branchManager;
    private Form m_form;

    // use to define all possible sort fields
    private enum SortField {
        USERNAME, LASTNAME, FIRSTNAME, NONE;

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

    // GET - Retrieve all and single User
    // ----------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        User user;
        UserRestInfoFull userRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                user = m_coreContext.getUser(parameterInfo.getValue());
                if (user == null) {
                    return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                            .getValue());
                }

                userRestInfo = createUserRestInfo(user);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new UserRepresentation(variant.getMediaType(), userRestInfo);
        }

        // if not single, process request for list
        List<User> users;
        Collection<Integer> userIds;

        IntParameterInfo parameterInfoBranchId =
                RestUtilities.getIntFromQueryString(m_form, REQUEST_ATTRIBUTE_BRANCH);
        IntListParameterInfo parameterInfoIdList =
                RestUtilities.getIntListFromQueryString(m_form, REQUEST_ATTRIBUTE_IDLIST);

        // check if searching by branch or id list
        if (parameterInfoBranchId.getExists()) {
            if (!parameterInfoBranchId.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, "Branch:"
                    + parameterInfoBranchId.getValueString());
            }

            userIds =
                    m_coreContext.getBranchMembersByPage(parameterInfoBranchId.getValue(), 0, m_coreContext
                            .getBranchMembersCount(parameterInfoBranchId.getValue()));
            users = getUsers(userIds);
        } else if (parameterInfoIdList.getExists()) {
            if (!parameterInfoIdList.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfoIdList
                        .getListString());
            }

            users = new ArrayList<User>();
            for (int id : parameterInfoIdList.getIntList()) {
                try {
                    user = m_coreContext.getUser(id);
                    users.add(user);
                } catch (Exception exception) {
                    return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, id);
                }
            }
        } else {
            // process request for all
            // no GetUsers() in coreContext, instead some subgroups
            users = m_coreContext.loadUsersByPage(1, m_coreContext.getAllUsersCount());
        }

        List<UserRestInfoFull> usersRestInfo = new ArrayList<UserRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort if specified
        sortUsers(users);

        // set requested items and get resulting metadata
        metadataRestInfo = addUsers(usersRestInfo, users);

        // create final restinfo
        UsersBundleRestInfo usersBundleRestInfo = new UsersBundleRestInfo(usersRestInfo, metadataRestInfo);

        return new UsersRepresentation(variant.getMediaType(), usersBundleRestInfo);
    }

    // PUT - Update or Add single User
    // -------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get from request body
        UserRepresentation representation = new UserRepresentation(entity);
        UserRestInfoFull userRestInfo = representation.getObject();
        User user;

        // validate input for update or create
        ValidationInfo validationInfo = validate(userRestInfo);

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

            // copy values over to existing item
            try {
                user = m_coreContext.getUser(parameterInfo.getValue());
                if (user == null) {
                    RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                    return;
                }

                updateUser(user, userRestInfo);
                m_coreContext.saveUser(user);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, user.getId());
            return;
        }

        // otherwise add new item
        try {
            user = createUser(userRestInfo);
            m_coreContext.saveUser(user);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, user.getId());
    }

    // DELETE - Delete single User
    // ---------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        User user;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                user = m_coreContext.getUser(parameterInfo.getValue());
                if (user == null) {
                    RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                    return;
                }

                // using deleteUser causes hibernate exception
                // "a different object with the same identifier value was already associated with the session"
                // m_coreContext.deleteUser(user);

                // use method for deleting implemented in sipxconfig ui
                Collection<Integer> userIds = new HashSet<Integer>();
                userIds.add(parameterInfo.getValue());
                Boolean deleteSuccess = m_coreContext.deleteUsers(userIds);

                if (!deleteSuccess) {
                    RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue());
                }

            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, user.getId());
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
    private ValidationInfo validate(UserRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String userName = restInfo.getUserName();

        validationInfo.checkString(userName, "UserName", StringConstraint.NOT_EMPTY);

        return validationInfo;
    }

    private UserRestInfoFull createUserRestInfo(User user) {
        UserRestInfoFull userRestInfo = null;
        UserGroupRestInfo userGroupRestInfo = null;
        List<UserGroupRestInfo> userGroupsRestInfo = new ArrayList<UserGroupRestInfo>();
        Set<Group> groups = null;
        BranchRestInfo branchRestInfo = null;
        Branch branch = null;
        AliasRestInfo aliasRestInfo = null;
        List<AliasRestInfo> aliasesRestInfo = new ArrayList<AliasRestInfo>();
        Set<String> aliases = null;

        groups = user.getGroups();

        // user does not necessarily have any groups
        if ((groups != null) && (!groups.isEmpty())) {
            for (Group group : groups) {
                userGroupRestInfo = new UserGroupRestInfo(group);
                userGroupsRestInfo.add(userGroupRestInfo);
            }
        }

        branch = user.getBranch();

        // user does not necessarily have branch
        if (branch != null) {
            branchRestInfo = new BranchRestInfo(branch);
        }

        aliases = user.getAliases();

        // user does not necessarily have any aliases
        if (aliases != null) {
            for (String alias : aliases) {
                aliasRestInfo = new AliasRestInfo(alias);
                aliasesRestInfo.add(aliasRestInfo);
            }
        }

        userRestInfo = new UserRestInfoFull(user, userGroupsRestInfo, branchRestInfo, aliasesRestInfo);

        return userRestInfo;
    }

    private MetadataRestInfo addUsers(List<UserRestInfoFull> usersRestInfo, List<User> users) {
        UserRestInfoFull userRestInfo;
        User user;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, users.size());

        // create list of skill restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            user = users.get(index);

            userRestInfo = createUserRestInfo(user);
            usersRestInfo.add(userRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortUsers(List<User> users) {
        // sort groups if requested
        SortInfo sortInfo = RestUtilities.calculateSorting(m_form);

        if (!sortInfo.getSort()) {
            return;
        }

        SortField sortField = SortField.toSortField(sortInfo.getSortField());
        boolean sortForward = sortInfo.getDirectionForward();

        switch (sortField) {
        case USERNAME:
            if (sortForward) {
                Collections.sort(users, new UserNameComparator());
            } else {
                Collections.sort(users, new ReverseComparator(new UserNameComparator()));
            }
            break;

        case LASTNAME:
            if (sortForward) {
                Collections.sort(users, new LastNameComparator());
            } else {
                Collections.sort(users, new ReverseComparator(new LastNameComparator()));
            }
            break;

        case FIRSTNAME:
            if (sortForward) {
                Collections.sort(users, new FirstNameComparator());
            } else {
                Collections.sort(users, new ReverseComparator(new FirstNameComparator()));
            }
            break;

        default:
            break;
        }
    }

    private List<User> getUsers(Collection<Integer> userIds) {
        List<User> users;

        users = new ArrayList<User>();
        for (int userId : userIds) {
            users.add(m_coreContext.getUser(userId));
        }

        return users;
    }

    private void updateUser(User user, UserRestInfoFull userRestInfo) {
        Branch branch;

        user.setUserName(userRestInfo.getUserName());

        // if pin is empty do not save
        if (!userRestInfo.getPin().isEmpty()) {
            user.setPin(userRestInfo.getPin());
        }

        // if sip password is empty, do not save
        if (!userRestInfo.getSipPassword().isEmpty()) {
            user.setSipPassword(userRestInfo.getSipPassword());
        }

        user.setLastName(userRestInfo.getLastName());
        user.setFirstName(userRestInfo.getFirstName());
        user.setEmailAddress(userRestInfo.getEmailAddress());

        // user may not have any groups
        List<UserGroupRestInfo> userGroupsRestInfo = userRestInfo.getGroups();
        if (userGroupsRestInfo != null) {
            user.setGroups(createUserGroups(userRestInfo));
        } else {
            user.setGroups(null);
        }

        // user may not have a branch
        if (userRestInfo.getBranch() != null) {
            branch = m_branchManager.getBranch(userRestInfo.getBranch().getId());
            user.setBranch(branch);
        } else {
            user.setBranch(null);
        }

        // user may not have any aliases
        if (userRestInfo.getAliases() != null) {
            user.setAliases(createAliases(userRestInfo));
        } else {
            user.setAliases(null);
        }
    }

    private User createUser(UserRestInfoFull userRestInfo) {
        User user = m_coreContext.newUser();

        // set first time credentials
        user.setPin(userRestInfo.getPin());
        user.setSipPassword(userRestInfo.getSipPassword());

        updateUser(user, userRestInfo);

        return user;
    }

    private Set<Group> createUserGroups(UserRestInfoFull userRestInfo) {
        Set<Group> userGroups = new TreeSet<Group>();
        Group userGroup;

        for (UserGroupRestInfo userGroupRestInfo : userRestInfo.getGroups()) {
            userGroup = m_coreContext.getGroupById(userGroupRestInfo.getId());
            userGroups.add(userGroup);
        }

        return userGroups;
    }

    private Set<String> createAliases(UserRestInfoFull userRestInfo) {
        Set<String> aliases = new LinkedHashSet<String>();

        for (AliasRestInfo aliasRestInfo : userRestInfo.getAliases()) {
            aliases.add(aliasRestInfo.getAlias());
        }

        return aliases;
    }

    // Sorting comparator classes
    // --------------------------

    private static class UserNameComparator implements Comparator<User> {
        @Override
        public int compare(User item1, User item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getUserName(), item2.getUserName());
        }
    }

    private static class LastNameComparator implements Comparator<User> {
        @Override
        public int compare(User item1, User item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getLastName(), item2.getLastName());
        }
    }

    private static class FirstNameComparator implements Comparator<User> {
        @Override
        public int compare(User item1, User item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getFirstName(), item2.getFirstName());
        }
    }

    // REST Representations
    // --------------------

    static class UsersRepresentation extends XStreamRepresentation<UsersBundleRestInfo> {

        public UsersRepresentation(MediaType mediaType, UsersBundleRestInfo object) {
            super(mediaType, object);
        }

        public UsersRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_USERBUNDLE, UsersBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_USER, UserRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_USERGROUP, UserGroupRestInfo.class);
            xstream.alias(ELEMENT_NAME_BRANCH, BranchRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_ALIAS, AliasRestInfo.class);
        }
    }

    static class UserRepresentation extends XStreamRepresentation<UserRestInfoFull> {

        public UserRepresentation(MediaType mediaType, UserRestInfoFull object) {
            super(mediaType, object);
        }

        public UserRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_USERGROUP, UserGroupRestInfo.class);
            xstream.alias(ELEMENT_NAME_USER, UserRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_BRANCH, BranchRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_ALIAS, AliasRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class UsersBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<UserRestInfoFull> m_users;

        public UsersBundleRestInfo(List<UserRestInfoFull> users, MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_users = users;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<UserRestInfoFull> getSkills() {
            return m_users;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }

}
