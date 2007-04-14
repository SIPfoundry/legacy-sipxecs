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
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.sipfoundry.sipxconfig.upload.UploadManager;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;

public abstract class ManageUploads extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "ManageUploads";

    public abstract void setUpload(Collection upload);

    public abstract Collection getUpload();

    public abstract UploadManager getUploadManager();

    public abstract UploadSpecification getSelectedSpecification();

    public abstract SelectMap getSelections();

    public IPage editUpload(IRequestCycle cycle, Integer uploadId) {
        EditUpload page = (EditUpload) cycle.getPage(EditUpload.PAGE);
        page.setUploadId(uploadId);
        page.setReturnPage(PAGE);
        return page;
    }

    public IPage addUpload(IRequestCycle cycle) {
        if (getSelectedSpecification() == null) {
            return null;
        }

        EditUpload page = (EditUpload) cycle.getPage(EditUpload.PAGE);
        page.setUploadId(null);
        page.setUploadSpecification(getSelectedSpecification());
        page.setReturnPage(PAGE);
        return page;
    }

    public void deleteUpload() {
        Upload[] uploads = (Upload[]) DaoUtils.loadBeansArrayByIds(getUploadManager(),
                Upload.class, getSelections().getAllSelected());
        for (int i = 0; i < uploads.length; i++) {
            getUploadManager().deleteUpload(uploads[i]);
        }
        // force reload
        setUpload(null);
    }

    public void activate() {
        setDeployed(true);
    }

    public void inactivate() {
        setDeployed(false);
    }

    private void setDeployed(boolean deploy) {
        Upload[] uploads = (Upload[]) DaoUtils.loadBeansArrayByIds(getUploadManager(),
                Upload.class, getSelections().getAllSelected());
        for (int i = 0; i < uploads.length; i++) {
            if (deploy) {
                getUploadManager().deploy(uploads[i]);
            } else {
                getUploadManager().undeploy(uploads[i]);
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
