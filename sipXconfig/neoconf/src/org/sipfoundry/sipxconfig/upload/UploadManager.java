/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.upload;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.DataObjectSource;

public interface UploadManager extends DataObjectSource<Upload> {

    static final String CONTEXT_BEAN_NAME = "uploadManager";

    Upload loadUpload(Integer uploadId);

    void saveUpload(Upload upload);

    void deleteUpload(Upload upload);

    void deleteUploads(Collection<Integer> uploadIds);

    Collection<Upload> getUpload();

    Upload newUpload(UploadSpecification manufacturer);

    boolean isActiveUploadById(UploadSpecification spec);

    UploadSpecification getSpecification(String specId);

    /**
     * Checks to ensure you're not deploying more than one type of upload then delegates
     * deployment to upload object then saves the upload state is deployment was successful
     */
    void deploy(Upload upload);

    /**
     * Find any Uploads of this specification and undeloy them
     */
    void undeploy(UploadSpecification spec);

    /**
     * Delegates undeployment to upload object then saves the upload state is undeployment was
     * successful
     */
    void undeploy(Upload upload);

    /**
     * testing only
     */
    void clear();
}
