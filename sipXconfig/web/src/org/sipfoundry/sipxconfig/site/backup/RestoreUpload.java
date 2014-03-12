/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.backup;


import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupType;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.common.AssetSelector;

public abstract class RestoreUpload extends BaseComponent implements PageBeginRenderListener {
    private static final String CANNOT_RESTORE_KEY = "&cannot.restore";

    @InjectObject(value = "spring:backupManager")
    public abstract BackupManager getBackupManager();

    public abstract String getDefinitionId();

    public abstract Map<String, IUploadFile> getUploads();

    public abstract void setUploads(Map<String, IUploadFile> uploads);

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Parameter(required = true)
    public abstract boolean isCanRestore();

    @InjectPage(value = RestoreFinalize.PAGE)
    public abstract RestoreFinalize getFinalizePage();

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getUploads() == null) {
            setUploads(new HashMap<String, IUploadFile>());
        }
        //on first page display, show informal message about whether restore could be possible
        if (!isCanRestore()) {
            getValidator().record(new UserException(CANNOT_RESTORE_KEY), getMessages());
        }
    }

    public IPage uploadAndRestoreFiles() {
        IValidationDelegate validator = TapestryUtils.getValidator(this);
        try {
            //make sure restore won't be initiated if validation do not pass
            if (!isCanRestore()) {
                getValidator().record(new UserException(CANNOT_RESTORE_KEY), getMessages());
                return null;
            }
            String[] ids = getBackupManager().getArchiveDefinitionIds().toArray(new String[0]);
            File dir = getBackupManager().getCleanRestoreStagingDirectory();
            Set<String> defs = new HashSet<String>();
            for (int i = 0; i < ids.length; i++) {
                IUploadFile upload = getUploads().get(ids[i]);
                if (upload != null) {
                    upload(dir, ids[i], upload);
                    defs.add(ids[i]);
                }
            }

            if (defs.isEmpty()) {
                throw new ValidatorException(getMessages().getMessage("message.noFileToRestore"));
            }

            // Q: Should this catch exception and delete files then rethrow exception?

            RestoreFinalize page = getFinalizePage();
            page.setBackupType(BackupType.local); // files are already retrieved so this is meaningless
            page.setUploadedIds(defs);
            List<String> none = Collections.emptyList();
            page.setSelections(none);
            page.setCallback(new PageCallback(getPage()));
            return page;
        } catch (ValidatorException e) {
            validator.record(e);
            return null;
        }
    }

    private File upload(File restoreDir, String defId, IUploadFile uploadFile) throws ValidatorException {
        String ext = ".tar.gz";
        if (uploadFile == null) {
            return null;
        }
        String fileName = AssetSelector.getSystemIndependentFileName(uploadFile.getFilePath());
        if (!fileName.endsWith(ext)) {
            String error = getMessages().getMessage("message.wrongFileToRestore");
            throw new ValidatorException(error);
        }

        OutputStream os = null;
        try {
            File archive = new File(restoreDir, defId);
            os = new FileOutputStream(archive);
            IOUtils.copy(uploadFile.getStream(), os);
            return archive;
        } catch (IOException ex) {
            String error = getMessages().getMessage("message.failed.uploadConfiguration");
            throw new ValidatorException(error);
        } finally {
            IOUtils.closeQuietly(os);
        }
    }
}
