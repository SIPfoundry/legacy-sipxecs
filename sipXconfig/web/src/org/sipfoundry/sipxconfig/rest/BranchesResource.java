/*
 *
 *  Copyright (C) 2012 PATLive, D. Chang
 *  Contributed to SIPfoundry under a Contributor Agreement
 *  BranchesResource.java - A Restlet to read Branch data from SipXecs
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
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.rest.RestUtilities.BranchRestInfoFull;
import org.sipfoundry.sipxconfig.rest.RestUtilities.IntParameterInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.MetadataRestInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.PaginationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.SortInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo;
import org.sipfoundry.sipxconfig.rest.RestUtilities.ValidationInfo.StringConstraint;
import org.springframework.beans.factory.annotation.Required;

import com.thoughtworks.xstream.XStream;

public class BranchesResource extends Resource {

    private static final String ELEMENT_NAME_BRANCH = "branch";
    private static final String ELEMENT_NAME_BRANCHBUNDLE = ELEMENT_NAME_BRANCH;

    private BranchManager m_branchManager;
    private Form m_form;

    // use to define all possible sort fields
    private enum SortField {
        NAME, DESCRIPTION, CITY, OFFICEDESIGNATION, NONE;

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

    // GET - Retrieve all and single Branch
    // ------------------------------------

    @Override
    public Representation represent(Variant variant) throws ResourceException {
        IntParameterInfo parameterInfo;
        Branch branch;
        BranchRestInfoFull branchRestInfo;

        // if have id then get a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                return RestUtilities.getResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo
                        .getValueString());
            }

            try {
                branch = m_branchManager.retrieveBranch(parameterInfo.getValue());
                if (branch == null) {
                    return RestUtilities.getResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo
                            .getValue());
                }

                branchRestInfo = createBranchRestInfo(branch);
            } catch (Exception exception) {
                return RestUtilities.getResponseError(getResponse(), ERROR_READ_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
            }

            return new BranchRepresentation(variant.getMediaType(), branchRestInfo);
        }

        // if not single, process request for list
        List<Branch> branches = m_branchManager.getBranches();
        List<BranchRestInfoFull> branchesRestInfo = new ArrayList<BranchRestInfoFull>();
        MetadataRestInfo metadataRestInfo;

        // sort if specified
        sortBranches(branches);

        // set requested agents groups and get resulting metadata
        metadataRestInfo = addBranches(branchesRestInfo, branches);

        // create final restinfo
        BranchesBundleRestInfo branchesBundleRestInfo =
                new BranchesBundleRestInfo(branchesRestInfo, metadataRestInfo);

        return new BranchesRepresentation(variant.getMediaType(), branchesBundleRestInfo);
    }

    // PUT - Update or Add single Branch
    // ---------------------------------

    @Override
    public void storeRepresentation(Representation entity) throws ResourceException {
        IntParameterInfo parameterInfo;

        // get item from request body
        BranchRepresentation representation = new BranchRepresentation(entity);
        BranchRestInfoFull branchRestInfo = representation.getObject();
        Branch branch;

        // validate input for update or create
        ValidationInfo validationInfo = validate(branchRestInfo);

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
                branch = m_branchManager.retrieveBranch(parameterInfo.getValue());
                if (branch == null) {
                    RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                    return;
                }

                // copy values over to existing item
                updateBranch(branch, branchRestInfo);
                m_branchManager.saveBranch(branch);
            } catch (Exception exception) {
                RestUtilities.setResponseError(getResponse(), ERROR_UPDATE_FAILED, parameterInfo.getValue(),
                        exception.getLocalizedMessage());
                return;
            }

            RestUtilities.setResponse(getResponse(), SUCCESS_UPDATED, branch.getId());
            return;
        }

        // if not single, add new item
        try {
            branch = createBranch(branchRestInfo);
            m_branchManager.saveBranch(branch);
        } catch (Exception exception) {
            RestUtilities.setResponseError(getResponse(), ERROR_CREATE_FAILED, exception.getLocalizedMessage());
            return;
        }

        RestUtilities.setResponse(getResponse(), SUCCESS_CREATED, branch.getId());
    }

    // DELETE - Delete single Branch
    // -----------------------------

    @Override
    public void removeRepresentations() throws ResourceException {
        IntParameterInfo parameterInfo;
        Branch branch;

        // get id then delete a single item
        parameterInfo = RestUtilities.getIntFromAttribute(getRequest(), REQUEST_ATTRIBUTE_ID);
        if (parameterInfo.getExists()) {
            if (!parameterInfo.getValid()) {
                RestUtilities.setResponseError(getResponse(), ERROR_ID_INVALID, parameterInfo.getValueString());
                return;
            }

            try {
                // do not need object to delete, but confirm existence for error message
                branch = m_branchManager.retrieveBranch(parameterInfo.getValue());
                if (branch == null) {
                    RestUtilities.setResponseError(getResponse(), ERROR_OBJECT_NOT_FOUND, parameterInfo.getValue());
                    return;
                }

                List<Integer> branchIds = new ArrayList<Integer>();
                branchIds.add(parameterInfo.getValue());
                m_branchManager.deleteBranches(branchIds);
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
    private ValidationInfo validate(BranchRestInfoFull restInfo) {
        ValidationInfo validationInfo = new ValidationInfo();

        String name = restInfo.getName();

        validationInfo.checkString(name, "Name", StringConstraint.NOT_EMPTY);

        return validationInfo;
    }

    private BranchRestInfoFull createBranchRestInfo(Branch branch) {
        BranchRestInfoFull branchRestInfo = null;

        branchRestInfo = new BranchRestInfoFull(branch);

        return branchRestInfo;
    }

    private MetadataRestInfo addBranches(List<BranchRestInfoFull> branchesRestInfo, List<Branch> branches) {
        Branch branch;
        BranchRestInfoFull branchRestInfo;

        // determine pagination
        PaginationInfo paginationInfo = RestUtilities.calculatePagination(m_form, branches.size());

        // create list of skill restinfos
        for (int index = paginationInfo.getStartIndex(); index <= paginationInfo.getEndIndex(); index++) {
            branch = branches.get(index);
            branchRestInfo = createBranchRestInfo(branch);
            branchesRestInfo.add(branchRestInfo);
        }

        // create metadata about agent groups
        MetadataRestInfo metadata = new MetadataRestInfo(paginationInfo);
        return metadata;
    }

    @SuppressWarnings("unchecked")
    private void sortBranches(List<Branch> branches) {
        // sort if requested
        SortInfo sortInfo = RestUtilities.calculateSorting(m_form);

        if (!sortInfo.getSort()) {
            return;
        }

        SortField sortField = SortField.toSortField(sortInfo.getSortField());
        boolean sortForward = sortInfo.getDirectionForward();

        switch (sortField) {
        case CITY:
            if (sortForward) {
                Collections.sort(branches, new CityComparator());
            } else {
                Collections.sort(branches, new ReverseComparator(new CityComparator()));
            }
            break;

        case OFFICEDESIGNATION:
            if (sortForward) {
                Collections.sort(branches, new OfficeDesignationComparator());
            } else {
                Collections.sort(branches, new ReverseComparator(new OfficeDesignationComparator()));
            }
            break;

        case NAME:
            if (sortForward) {
                Collections.sort(branches, new NameComparator());
            } else {
                Collections.sort(branches, new ReverseComparator(new NameComparator()));
            }
            break;

        case DESCRIPTION:
            if (sortForward) {
                Collections.sort(branches, new DescriptionComparator());
            } else {
                Collections.sort(branches, new ReverseComparator(new DescriptionComparator()));
            }
            break;

        default:
            break;
        }
    }

    private void updateBranch(Branch branch, BranchRestInfoFull branchRestInfo) {
        Address address;

        branch.setName(branchRestInfo.getName());
        branch.setDescription(branchRestInfo.getDescription());
        branch.setPhoneNumber(branchRestInfo.getPhoneNumber());
        branch.setFaxNumber(branchRestInfo.getFaxNumber());

        address = getAddress(branchRestInfo);
        branch.setAddress(address);
    }

    private Branch createBranch(BranchRestInfoFull branchRestInfo) throws ResourceException {
        Branch branch = new Branch();

        updateBranch(branch, branchRestInfo);

        return branch;
    }

    private Address getAddress(BranchRestInfoFull branchRestInfo) {
        Address address = new Address();

        address.setCity(branchRestInfo.getAddress().getCity());
        address.setCountry(branchRestInfo.getAddress().getCountry());
        address.setOfficeDesignation(branchRestInfo.getAddress().getOfficeDesignation());
        address.setState(branchRestInfo.getAddress().getState());
        address.setStreet(branchRestInfo.getAddress().getStreet());
        address.setZip(branchRestInfo.getAddress().getZip());

        return address;
    }

    private static class CityComparator implements Comparator<Branch> {
        @Override
        public int compare(Branch branch1, Branch branch2) {
            return RestUtilities.compareIgnoreCaseNullSafe(branch1.getAddress().getCity(), branch2.getAddress()
                    .getCity());
        }
    }

    // Sorting comparator classes
    // --------------------------

    private static class OfficeDesignationComparator implements Comparator<Branch> {
        @Override
        public int compare(Branch item1, Branch item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getAddress().getOfficeDesignation(), item2
                    .getAddress().getOfficeDesignation());
        }
    }

    private static class NameComparator implements Comparator<Branch> {
        @Override
        public int compare(Branch item1, Branch item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getName(), item2.getName());
        }
    }

    private static class DescriptionComparator implements Comparator<Branch> {
        @Override
        public int compare(Branch item1, Branch item2) {
            return RestUtilities.compareIgnoreCaseNullSafe(item1.getDescription(), item2.getDescription());
        }
    }

    // REST Representations
    // --------------------

    static class BranchesRepresentation extends XStreamRepresentation<BranchesBundleRestInfo> {

        public BranchesRepresentation(MediaType mediaType, BranchesBundleRestInfo object) {
            super(mediaType, object);
        }

        public BranchesRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_BRANCHBUNDLE, BranchesBundleRestInfo.class);
            xstream.alias(ELEMENT_NAME_BRANCH, BranchRestInfoFull.class);
        }
    }

    static class BranchRepresentation extends XStreamRepresentation<BranchRestInfoFull> {

        public BranchRepresentation(MediaType mediaType, BranchRestInfoFull object) {
            super(mediaType, object);
        }

        public BranchRepresentation(Representation representation) {
            super(representation);
        }

        @Override
        protected void configureXStream(XStream xstream) {
            xstream.alias(ELEMENT_NAME_BRANCH, BranchRestInfoFull.class);
        }
    }

    // REST info objects
    // -----------------

    static class BranchesBundleRestInfo {
        private final MetadataRestInfo m_metadata;
        private final List<BranchRestInfoFull> m_branches;

        public BranchesBundleRestInfo(List<BranchRestInfoFull> branches, MetadataRestInfo metadata) {
            m_metadata = metadata;
            m_branches = branches;
        }

        public MetadataRestInfo getMetadata() {
            return m_metadata;
        }

        public List<BranchRestInfoFull> getBranches() {
            return m_branches;
        }
    }

    // Injected objects
    // ----------------

    @Required
    public void setBranchManager(BranchManager branchManager) {
        m_branchManager = branchManager;
    }

}
