/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.upload;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.sipfoundry.sipxconfig.upload.UploadManager;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;

public abstract class EditUpload extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "upload/EditUpload";

    @InjectObject(value = "spring:uploadManager")
    public abstract UploadManager getUploadManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract Upload getUpload();

    public abstract void setUpload(Upload upload);

    @Persist
    public abstract Integer getUploadId();

    public abstract void setUploadId(Integer id);

    @Persist
    public abstract UploadSpecification getUploadSpecification();

    public abstract void setUploadSpecification(UploadSpecification specification);

    public abstract boolean isActive();

    public abstract void setActive(boolean deployed);

    public void pageBeginRender(PageEvent event_) {
        Upload upload = getUpload();
        if (upload == null) {
            Integer id = getUploadId();
            if (id == null) {
                upload = getUploadManager().newUpload(getUploadSpecification());
            } else {
                upload = getUploadManager().loadUpload(id);
            }
            setUpload(upload);
            setActive(upload.isDeployed());
        }
    }

    private void checkDeploymentStatus() {
        Upload upload = getUpload();
        if (isActive() != upload.isDeployed()) {
            if (!isActive()) {
                getUploadManager().undeploy(upload);
            } else if (isActive()) {
                getUploadManager().deploy(upload);
            }
            setActive(upload.isDeployed());
        }
    }

    public void onSave() {
        if (TapestryUtils.isValid(this)) {
            Upload upload = getUpload();
            getUploadManager().saveUpload(upload);
            setUploadId(upload.getId());
            checkDeploymentStatus();
        }
    }
}
