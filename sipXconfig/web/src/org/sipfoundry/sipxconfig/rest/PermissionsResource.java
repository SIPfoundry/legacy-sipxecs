/*
 *
 *  Copyright (C) 2012 PATLive, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  PermissionsResource.java - A Restlet to read Skill data from SipXecs
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
import static org.sipfoundry.sipxconfig.rest.RestUtilities.REQUEST_ATTRIBUTE_NAME;
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
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PermissionRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.StringParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class PermissionsResource extends Resource {

    private static final String ELEMENT_NAME_PERMISSIONBUNDLE = "permissions";
    private static final String ELEMENT_NAME_PERMISSION = "permission";

    private PermissionManager m_permissionManager;
    private Form m_form;

    // use to define all possible sort fields
    private enum SortField {
        NAME, DESCRIPTION, LABEL, DEFAULTVALUE, NONE;

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
        Permission permission;
        StringParameterInfo parameterInfo;

        // Permissions do not use Id, so must key off Name
        PermissionRestInfoFull permissionRestInfo = null;

        // if have name then get a single item
        parameterInfo = RestUtilities.getStringFromAttribute(getRequest(), REQUEST_ATTRIBUTE_NAME);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValue());
            }

            try {
                permission = m_permissionManager.getPermissionByName(parameterInfo.getValue());
                if (permission == null) {
                    return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                            .getValue());
                }

                permissionRestInfo = createPermissionRestInfo(permission);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new PermissionRepresentation(variant.getMediaType(), permissionRestInfo);
        }

        // if not single, process request for list
        List<Permission> permissions = new ArrayList<Permission>(m_permissionManager.getPermissions());
        List<PermissionRestInfoFull> permissionsRestInfo = new ArrayList<PermissionRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort groups if specified
        sortPermissions(permissions);

        // set requested agents groups and get resulting metadata
        metadataRestInfo = addPermissions(permissionsRestInfo, permissions);

        // create final restinfo
        PermissionsBundleRestInfo permissionsBundleRestInfo =
                new PermissionsBundleRestInfo(permissionsRestInfo, metadataRestInfo);

        return new PermissionsRepresentation(variant.getMediaType(), permissionsBundleRestInfo);
    }

    // PUT - Update or Add single Skill
    // --------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        StringParameterInfo parameterInfo;

        // get from request body
        PermissionRepresentation representation = new PermissionRepresentation(entity);
        PermissionRestInfoFull permissionRestInfo = representation.getObject();
        Permission permission;

        // validate input for update or create
        ValidationInfo validationInfo = validate(permissionRestInfo);

        if (!validationInfo.getValid()) {
            RestUtilities.setResponseError(getResponse(), validationInfo.getResponseCode(), validationInfo
                    .getMessage());
            return;
        }

        // if have name then get a single item
        parameterInfo = RestUtilities.getStringFromAttribute(getRequest(), REQUEST_ATTRIBUTE_NAME);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValue());
                return;
            }

            // copy values over to existing item
            try {
                permission = m_permissionManager.getPermissionByName(parameterInfo.getValue());
                if (permission == null) {
                    RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                    return;
                }

                updatePermission(permission, permissionRestInfo);
                m_permissionManager.saveCallPermission(permission);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, permission.getName());
            return;
        }

        // if not single, add new item
        try {
            permission = createPermission(permissionRestInfo);
            m_permissionManager.saveCallPermission(permission);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, permission.getName());
    }

    // DELETE - Delete single Skill
    // ----------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        StringParameterInfo parameterInfo;
        Permission permission;

        // get name then delete single item
        parameterInfo = RestUtilities.getStringFromAttribute(getRequest(), REQUEST_ATTRIBUTE_NAME);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValue());
                return;
            }

            try {
                permission = m_permissionManager.getPermissionByName(parameterInfo.getValue());
                if (permission == null) {
                    RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                    return;
                }

                m_permissionManager.deleteCallPermission(permission);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_DELETE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_DELETED, permission.getName());
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
    private ValidationInfo validate(PermissionRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String label = restInfo.getLabel();

        validationInfo.checkString(label, "Label", StringConstraint.NOT_EMPTY);

        return validationInfo;
    }

    private PermissionRestInfoFull createPermissionRestInfo(Permission permission) {
        PermissionRestInfoFull permissionRestInfo = null;

        permissionRestInfo = new PermissionRestInfoFull(permission);

        return permissionRestInfo;
    }

    private MetadataRestInfo addPermissions(List<PermissionRestInfoFull> permissionsRestInfo,
            List<Permission> permissions) {
        Permission permission;
        PermissionRestInfoFull permissionRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, permissions.size());

        // create list of restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            permission = permissions.get(index);
            permissionRestInfo = createPermissionRestInfo(permission);
            permissionsRestInfo.add(permissionRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortPermissions(List<Permission> permissions) {
        // sort if requested
        SortInfo sortInfo = RestUtilities.calculateSorting(m_form);

        if (!sortInfo.getSort()) {
            return;
        }

        SortField sortField = SortField.toSortField(sortInfo.getSortField());
        boolean sortForward = sortInfo.getDirectionForward();

        switch (sortField) {
        case LABEL:
            if (sortForward) {
                Collections.sort(permissions, new LabelComparator());
            } else {
                Collections.sort(permissions, new ReverseComparator(new LabelComparator()));
            }
            break;

        case DEFAULTVALUE:
            if (sortForward) {
                Collections.sort(permissions, new DefaultValueComparator());
            } else {
                Collections.sort(permissions, new ReverseComparator(new DefaultValueComparator()));
            }
            break;

        case NAME:
            if (sortForward) {
                Collections.sort(permissions, new NameComparator());
            } else {
                Collections.sort(permissions, new ReverseComparator(new NameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(permissions, new DescriptionComparator());
            } else {
                Collections.sort(permissions, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updatePermission(Permission permission, PermissionRestInfoFull permissionRestInfo) {
        permission.setLabel(permissionRestInfo.getLabel());
        permission.setDescription(permissionRestInfo.getDescription());
        permission.setDefaultValue(permissionRestInfo.getDefaultValue());
    }

    private Permission createPermission(PermissionRestInfoFull permissionRestInfo) throws ResourceException {
        Permission permission = new Permission();

        // only available is custom call types (cannot change so no update)
        permission.setType(Permission.Type.CALL);

        updatePermission(permission, permissionRestInfo);

        return permission;
    }

    // Sorting comparator classes
    // --------------------------

    private static class LabelComparator implements Comparator<Permission> {
        @Override
        public int compare(Permission item1, Permission item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getLabel(), item2.getLabel());
        }
    }

    private static class DefaultValueComparator implements Comparator<Permission> {
        @Override
        public int compare(Permission item1, Permission item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(Boolean.toString(item1.getDefaultValue()), Boolean
                    .toString(item2.getDefaultValue()));
        }
    }

    private static class NameComparator implements Comparator<Permission> {
        @Override
        public int compare(Permission item1, Permission item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class DescriptionComparator implements Comparator<Permission> {
        @Override
        public int compare(Permission item1, Permission item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    // REST Representations
    // --------------------

    static class PermissionsRepresentation extends XStreamRepresentation<PermissionsBundleRestInfo> {

        public PermissionsRepresentation(MediaType mediaType, PermissionsBundleRestInfo object) {
            super(mediaType, object);
        }

        public PermissionsRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_PERMISSIONBUNDLE, PermissionsBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_PERMISSION, PermissionRestInfoFull.class);
        }
    }

    static class PermissionRepresentation extends XStreamRepresentation<PermissionRestInfoFull> {

        public PermissionRepresentation(MediaType mediaType, PermissionRestInfoFull object) {
            super(mediaType, object);
        }

        public PermissionRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_PERMISSION, PermissionRestInfoFull.class);
        }
    }

    // REST info objects
    // -----------------

    static class PermissionsBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<PermissionRestInfoFull> m_permissions;

        public PermissionsBundleRestInfo(List<PermissionRestInfoFull> permissions, MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_permissions = permissions;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<PermissionRestInfoFull> getPermissions() {
            return m_permissions;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }
}
