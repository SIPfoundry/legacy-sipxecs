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

import java.util.Collection;

import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Message;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.upload.DefaultSystemFirmwareInstall;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.sipfoundry.sipxconfig.upload.UploadManager;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;

public abstract class ManageUploads extends SipxBasePage implements PageBeginRenderListener {

    public static final String PAGE = "upload/ManageUploads";

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectObject(value = "spring:uploadManager")
    public abstract UploadManager getUploadManager();

    @InjectObject(value = "spring:defaultSystemFirmwareInstall")
    public abstract DefaultSystemFirmwareInstall getDefaultSystemFirmwareInstall();

    @InjectObject(value = "spring:uploadSpecificationSource")
    public abstract ModelSource<UploadSpecification> getUploadSpecificationSource();

    @InjectPage(value = EditUpload.PAGE)
    public abstract EditUpload getEditUploadPage();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    public abstract Upload getCurrentRow();

    public abstract void setUpload(Collection upload);

    public abstract Collection getUpload();

    public abstract UploadSpecification getSelectedSpecification();

    @Message("error.alreadyActivated")
    public abstract String getAlreadyActivatedError();

    public IPage editUpload(Integer uploadId) {
        EditUpload page = getEditUploadPage();
        page.setUploadId(uploadId);
        page.setReturnPage(PAGE);
        return page;
    }

    public IPage addUpload() {
        if (getSelectedSpecification() == null) {
            return null;
        }

        EditUpload page = getEditUploadPage();
        page.setUploadId(null);
        page.setUploadSpecification(getSelectedSpecification());
        page.setReturnPage(PAGE);
        return page;
    }

    public void deleteUpload() {
        getUploadManager().deleteUploads(getSelections().getAllSelected());
        // force reload
        setUpload(null);
    }

    public void activate() {
        setDeployed(true);
    }

    public void inactivate() {
        setDeployed(false);
    }

    public void installFirmware() {
        // force reload
        setUpload(null);
        getDefaultSystemFirmwareInstall().installAvailableFirmwares();
    }

    private void setDeployed(boolean deploy) {
        Upload[] uploads = DaoUtils.loadBeansArrayByIds(getUploadManager(), Upload.class, getSelections()
                .getAllSelected());
        for (Upload upload : uploads) {
            if (deploy) {
                UploadSpecification uploadSpec = upload.getSpecification();
                boolean alreadyActive = upload.isDeployed();

                // Managed device types only support a single active entry in the device files
                // table.
                if (uploadSpec.getManaged()) {
                    alreadyActive = getUploadManager().isActiveUploadById(uploadSpec);
                }

                if (alreadyActive) {
                    getValidator().record(getAlreadyActivatedError(), null);
                } else {
                    getUploadManager().deploy(upload);
                }
            } else {
                getUploadManager().undeploy(upload);
            }
        }
        // force reload
        setUpload(null);
    }

    /** stub: side-effect of PageBeginRenderListener */
    public void pageBeginRender(PageEvent event_) {
        if (getUpload() == null) {
            setUpload(getUploadManager().getUpload());
        }
    }
}
