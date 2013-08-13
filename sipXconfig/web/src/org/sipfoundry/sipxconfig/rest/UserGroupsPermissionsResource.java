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
import static org.sipfoundry.sipxconfig.rest.RestUtilities.REQUEST_ATTRIBUTE_ID;
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
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SettingPermissionRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.UserGroupPermissionRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class UserGroupsPermissionsResource extends Resource {

    private static final String ELEMENT_NAME_USERGROUPPERMISSIONBUNDLE = "user-group-permission";
    private static final String ELEMENT_NAME_USERGROUPPERMISSION = "group";
    private static final String ELEMENT_NAME_SETTINGPERMISSION = "setting";

    private CoreContext m_coreContext;
    private SettingDao m_settingContext; // saveGroup is not available through corecontext
    private PermissionManager m_permissionManager;
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
        return false;
    }

    // GET - Retrieve all and single User Group with Permissions
    // ---------------------------------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        Group userGroup;
        UserGroupPermissionRestInfoFull userGroupPermissionRestInfo;

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
                userGroupPermissionRestInfo = createUserGroupPermissionRestInfo(userGroup);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new UserGroupPermissionRepresentation(variant.getMediaType(), userGroupPermissionRestInfo);
        }

        // if not single, process request for list
        List<Group> userGroups = m_coreContext.getGroups();
        List<UserGroupPermissionRestInfoFull> userGroupPermissionsRestInfo =
                new ArrayList<UserGroupPermissionRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort if specified
        sortUserGroups(userGroups);

        // set requested items and get resulting metadata
        metadataRestInfo = addUserGroups(userGroupPermissionsRestInfo, userGroups);

        // create final restinfo
        UserGroupPermissionsBundleRestInfo userGroupPermissionsBundleRestInfo =
                new UserGroupPermissionsBundleRestInfo(userGroupPermissionsRestInfo, metadataRestInfo);

        return new UserGroupPermissionsRepresentation(variant.getMediaType(), userGroupPermissionsBundleRestInfo);
    }

    // PUT - Update Permissions
    // ------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get from request body
        UserGroupPermissionRepresentation representation = new UserGroupPermissionRepresentation(entity);
        UserGroupPermissionRestInfoFull userGroupPermissionRestInfo = representation.getObject();
        Group userGroup;

        // validate input for update or create
        ValidationInfo validationInfo = validate(userGroupPermissionRestInfo);

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
                updateUserGroupPermission(userGroup, userGroupPermissionRestInfo);
                m_coreContext.saveGroup(userGroup);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, userGroup.getId());
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
    private ValidationInfo validate(UserGroupPermissionRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        return validationInfo;
    }

    private UserGroupPermissionRestInfoFull createUserGroupPermissionRestInfo(Group group) {
        UserGroupPermissionRestInfoFull userGroupPermissionRestInfo = null;
        List<SettingPermissionRestInfo> settings;

        settings = createSettingsRestInfo(group);
        userGroupPermissionRestInfo = new UserGroupPermissionRestInfoFull(group, settings);

        return userGroupPermissionRestInfo;
    }

    private List<SettingPermissionRestInfo> createSettingsRestInfo(Group group) {
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
                permissionValue = group.getSettingValue(permission.getSettingPath());
            } catch (Exception exception) {
                permissionValue = "GetSettingValue error: " + exception.getLocalizedMessage();
            }

            defaultValue = permission.getDefaultValue();

            // if value is at default get empty string. but updating with empty string
            // will cause errors (caught in update function).
            // this is also different than user permissions, which always display
            // a value even if default, so make consistent.
            permissionValue = getNonEmptySettingValue(permissionValue, defaultValue);

            settingRestInfo =
                    new SettingPermissionRestInfo(permissionName, permissionLabel, permissionValue, defaultValue);
            settings.add(settingRestInfo);
        }

        return settings;
    }

    private MetadataRestInfo addUserGroups(List<UserGroupPermissionRestInfoFull> userGroupPermissionsRestInfo,
            List<Group> userGroups) {
        Group userGroup;
        UserGroupPermissionRestInfoFull userGroupPermissionRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, userGroups.size());

        // create list of restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            userGroup = userGroups.get(index);
            userGroupPermissionRestInfo = createUserGroupPermissionRestInfo(userGroup);
            userGroupPermissionsRestInfo.add(userGroupPermissionRestInfo);
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

    private void updateUserGroupPermission(Group userGroup,
            UserGroupPermissionRestInfoFull userGroupPermissionRestInfo) {
        Permission permission;

        // update each permission setting
        for (SettingPermissionRestInfo settingRestInfo : userGroupPermissionRestInfo.getPermissions()) {
            permission = m_permissionManager.getPermissionByName(settingRestInfo.getName());

            // if value does not exist setSettingValue will blank the value causing errors.
            // Instead userGroup.setSettingValue(setting, value, defaultValue) reverts
            // the setting to default if blank.
            // However, this requires a Setting object that I believe is obtained from
            // userGroup.inherhitSettingsForEditing, except I do not know how
            // to get access to the Bean to pass to the function
            String settingValue =
                    getNonEmptySettingValue(settingRestInfo.getValue(), settingRestInfo.getDefaultValue());
            userGroup.setSettingValue(permission.getSettingPath(), settingValue);
        }
    }

    private String getNonEmptySettingValue(String reportedSettingValue, Boolean defaultValue) {
        if ((reportedSettingValue == null) || (reportedSettingValue == "")) {
            if (defaultValue) {
                return "ENABLE";
            } else {
                return "DISABLE";
            }
        } else {
            return reportedSettingValue;
        }
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

    static class UserGroupPermissionsRepresentation extends
            XStreamRepresentation<UserGroupPermissionsBundleRestInfo> {

        public UserGroupPermissionsRepresentation(MediaType mediaType, UserGroupPermissionsBundleRestInfo object) {
            super(mediaType, object);
        }

        public UserGroupPermissionsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_USERGROUPPERMISSIONBUNDLE, UserGroupPermissionsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_USERGROUPPERMISSION, UserGroupPermissionRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_SETTINGPERMISSION, SettingPermissionRestInfo.class);
        }
    }

    static class UserGroupPermissionRepresentation extends XStreamRepresentation<UserGroupPermissionRestInfoFull> {

        public UserGroupPermissionRepresentation(MediaType mediaType, UserGroupPermissionRestInfoFull object) {
            super(mediaType, object);
        }

        public UserGroupPermissionRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_USERGROUPPERMISSION, UserGroupPermissionRestInfoFull.class);
            xstream.alias(ELEMENT_NAME_SETTINGPERMISSION, SettingPermissionRestInfo.class);
        }
    }

    // REST info objects
    // -----------------

    static class UserGroupPermissionsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<UserGroupPermissionRestInfoFull> m_groups;

        public UserGroupPermissionsBundleRestInfo(List<UserGroupPermissionRestInfoFull> userGroupPermissions,
                MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_groups = userGroupPermissions;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<UserGroupPermissionRestInfoFull> getGroups() {
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
    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

}
