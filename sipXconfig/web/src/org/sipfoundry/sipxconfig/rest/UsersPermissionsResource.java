/*
 *
 *  Copyright (C) 2012 PATLive, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  UserGroupPermissionsResource.java - A Restlet to read User Group data with Permissions from SipXecs
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
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_ID_INVALID;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_MISSING_ID;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_OBJECT_NOT_FOUND;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_READ_FAILED;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.ERROR_UPDATE_FAILED;
import static org.sipfoundry.sipxconfig.rest.RestUtilities.ResponseCode.SUCCESS_UPDATED;

import java.util.ArrayList;
import java.util.Collection;
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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntListParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SettingPermissionRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.UserPermissionRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class UsersPermissionsResource extends Resource {

    private static final String ELEMENT_NAME_USERPERMISSIONBUNDLE = "user-permission";
    private static final String ELEMENT_NAME_USERPERMISSION = "user";
    private static final String ELEMENT_NAME_SETTINGPERMISSION = "setting";

    private CoreContext m_coreContext;
    private PermissionManager m_permissionManager;
    private Form m_form;

    // use to define all possible sort fields
    private enum SortField {
        LASTNAME, FIRSTNAME, NONE;

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
        return false;
    }

    // GET - Retrieve all and single User with Permissions
    // ---------------------------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        User user;
        UserPermissionRestInfoFull userPermissionRestInfo;

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

                userPermissionRestInfo = createUserPermissionRestInfo(user);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new UserPermissionRepresentation(variant.getMediaType(), userPermissionRestInfo);
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

        List<UserPermissionRestInfoFull> userPermissionsRestInfo = new ArrayList<UserPermissionRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort if specified
        sortUsers(users);

        // set requested items and get resulting metadata
        metadataRestInfo = addUsers(userPermissionsRestInfo, users);

        // create final restinfo
        UserPermissionsBundleRestInfo userPermissionsBundleRestInfo =
                new UserPermissionsBundleRestInfo(userPermissionsRestInfo, metadataRestInfo);

        return new UserPermissionsRepresentation(variant.getMediaType(), userPermissionsBundleRestInfo);
    }

    // PUT - Update Permissions
    // ------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get from request body
        UserPermissionRepresentation representation = new UserPermissionRepresentation(entity);
        UserPermissionRestInfoFull userPermissionRestInfo = representation.getObject();
        User user;

        // validate input for update or create
        ValidationInfo validationInfo = validate(userPermissionRestInfo);

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

                updateUserPermission(user, userPermissionRestInfo);
                m_coreContext.saveUser(user);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, user.getId());
            return;
        }

        // otherwise error, since no creation of new permissions
        RestUtilities.setResponseError(getResponse(), ERROR_MISSING_ID);
    }

    // Helper functions
    // ----------------

    // basic interface level validation of data provided through REST interface for creation or
    // update
    // may also contain clean up of input data
    // may create another validation function if different rules needed for update v. create
    private ValidationInfo validate(UserPermissionRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        return validationInfo;
    }

    private UserPermissionRestInfoFull createUserPermissionRestInfo(User user) {
        UserPermissionRestInfoFull userPermissionRestInfo = null;
        List<SettingPermissionRestInfo> settings;

        settings = createSettingsRestInfo(user);
        userPermissionRestInfo = new UserPermissionRestInfoFull(user, settings);

        return userPermissionRestInfo;
    }

    private List<SettingPermissionRestInfo> createSettingsRestInfo(User user) {
        List<SettingPermissionRestInfo> settings = new ArrayList<SettingPermissionRestInfo>();
        SettingPermissionRestInfo settingRestInfo = null;
        Collection<Permission> permissions;
        String permissionName;
        String permissionLabel;
        String permissionValue;
        boolean defaultValue;

        permissions = m_permissionManager.getPermissions();

        // settings value for permissions are ENABLE or DISABLE instead of boolean
        for (Permission permission : permissions) {
            permissionName = permission.getName();
            permissionLabel = permission.getLabel();

            try {
                // empty return means setting is at default (unless error in input to
                // getSettingValue)
                permissionValue = user.getSettingValue(permission.getSettingPath());
            } catch (Exception exception) {
                permissionValue = "GetSettingValue error: " + exception.getLocalizedMessage();
            }

            defaultValue = permission.getDefaultValue();

            settingRestInfo =
                    new SettingPermissionRestInfo(permissionName, permissionLabel, permissionValue, defaultValue);
            settings.add(settingRestInfo);
        }

        return settings;
    }

    private MetadataRestInfo addUsers(List<UserPermissionRestInfoFull> userPermissionsRestInfo, List<User> users) {
        User user;
        UserPermissionRestInfoFull userPermissionRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, users.size());

        // create list of restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            user = users.get(index);
            userPermissionRestInfo = createUserPermissionRestInfo(user);
            userPermissionsRestInfo.add(userPermissionRestInfo);
        }

        // create metadata about restinfos
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortUsers(List<User> users) {
        // sort if requested
        SortInfo sortInfo = RestUtilities.calculateSorting(m_form);

        if (!sortInfo.getSort()) {
            return;
        }

        SortField sortField = SortField.toSortField(sortInfo.getSortField());
        boolean sortForward = sortInfo.getDirectionForward();

        switch (sortField) {
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

    private void updateUserPermission(User user, UserPermissionRestInfoFull userPermissionRestInfo) {
        Permission permission;

        // update each permission setting
        for (SettingPermissionRestInfo settingRestInfo : userPermissionRestInfo.getPermissions()) {
            permission = m_permissionManager.getPermissionByName(settingRestInfo.getName());
            user.setSettingValue(permission.getSettingPath(), settingRestInfo.getValue());
        }
    }

    // Sorting comparator classes
    // --------------------------

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

    static class UserPermissionsRepresentation extends XStreamRepresentation<UserPermissionsBundleRestInfo> {

        public UserPermissionsRepresentation(MediaType mediaType, UserPermissionsBundleRestInfo object) {
            super(mediaType, object);
        }

        public UserPermissionsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_USERPERMISSIONBUNDLE, UserPermissionsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_USERPERMISSION, UserPermissionRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_SETTINGPERMISSION, SettingPermissionRestInfo.class);
        }
    }

    static class UserPermissionRepresentation extends XStreamRepresentation<UserPermissionRestInfoFull> {

        public UserPermissionRepresentation(MediaType mediaType, UserPermissionRestInfoFull object) {
            super(mediaType, object);
        }

        public UserPermissionRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_USERPERMISSION, UserPermissionRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_SETTINGPERMISSION, SettingPermissionRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class UserPermissionsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<UserPermissionRestInfoFull> m_users;

        public UserPermissionsBundleRestInfo(List<UserPermissionRestInfoFull> userPermissions,
                MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_users = userPermissions;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<UserPermissionRestInfoFull> getUsers() {
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
    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }
}
