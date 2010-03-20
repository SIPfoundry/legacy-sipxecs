/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.OutputStream;
import java.io.Writer;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.LogFactory;
import org.apache.hivemind.Messages;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.bulk.csv.BulkManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;


public abstract class BulkImport extends SipxBasePage {
    @Persist
    @InitialValue(value = "literal:import")
    public abstract String getTab();

    public abstract IUploadFile getUploadFile();

    public abstract BulkManager getBulkManager();

    public abstract AdminContext getAdminContext();

    public abstract String getExportFile();

    public abstract void setExportFile(String filePath);

    public void submit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils
                .getValidator(this);
        IUploadFile uploadFile = getUploadFile();
        String filePath = uploadFile.getFilePath();
        if (StringUtils.isBlank(filePath)) {
            validator.record(getMessages().getMessage("msg.errorNoFile"), ValidationConstraint.REQUIRED);
            return;
        }
        OutputStream os = null;
        try {
            File tmpFile = File.createTempFile("csv_import", null);
            os = new FileOutputStream(tmpFile);
            IOUtils.copy(uploadFile.getStream(), os);
            os.close();
            getBulkManager().insertFromCsv(tmpFile, true);
            validator.recordSuccess(getMessages().getMessage("msg.success"));
        } catch (IOException e) {
            LogFactory.getLog(getClass()).error("Cannot import file", e);
            Messages messages = getMessages();
            validator.record(messages.format("msg.error", e.getLocalizedMessage()),
                    ValidationConstraint.CONSISTENCY);
        } finally {
            IOUtils.closeQuietly(os);
        }
    }

    public void export() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils.getValidator(this);

        try {

            File exportFile = File.createTempFile("export", ".csv");
            exportFile.deleteOnExit();
            Writer writer = new FileWriter(exportFile);
            getAdminContext().performExport(writer);
            writer.close();
            setExportFile(exportFile.getPath());
            validator.recordSuccess(getMessages().getMessage("msg.exportSuccess"));
        } catch (IOException e) {
            LogFactory.getLog(getClass()).error("Cannot create export file", e);
            Messages messages = getMessages();
            validator.record(messages.format("msg.exportError", e.getLocalizedMessage()),
                    ValidationConstraint.CONSISTENCY);
        }
    }
}
