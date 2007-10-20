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

import org.apache.commons.io.FilenameUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.hivemind.Messages;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.AssetSelector;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class LocalizationPage extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/LocalizationPage";

    private static final String LABEL = "label.";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject(value = "spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();

    public abstract IPropertySelectionModel getRegionList();

    public abstract void setRegionList(IPropertySelectionModel regionList);

    public abstract IPropertySelectionModel getLanguageList();

    public abstract void setLanguageList(IPropertySelectionModel languageList);

    public abstract String getRegion();

    public abstract String getLanguage();

    public abstract void setRegion(String region);

    public abstract void setLanguage(String language);

    public abstract IUploadFile getUploadFile();

    public void pageBeginRender(PageEvent event_) {
        if (getRegionList() == null) {
            initRegions();
        }

        if (getLanguageList() == null) {
            initLanguages();
        }

        if (getRegion() == null) {
            String defaultRegion = getLocalizationContext().getLocalization().getRegion();
            setRegion(defaultRegion);
        }

        if (getLanguage() == null) {
            String defaultLanguage = getLocalizationContext().getLocalization().getLanguage();
            setLanguage(defaultLanguage);
        }
    }

    private void initRegions() {
        String[] regions = getLocalizationContext().getInstalledRegions();
        IPropertySelectionModel model = new ModelWithDefaults(getMessages(), regions);
        setRegionList(model);
    }

    private void initLanguages() {
        String[] languages = getLocalizationContext().getInstalledLanguages();
        IPropertySelectionModel model = new ModelWithDefaults(getMessages(), languages);
        setLanguageList(model);
    }

    private static class ModelWithDefaults extends LocalizedOptionModelDecorator {
        public static final String DEFAULT = "default";

        public ModelWithDefaults(Messages messages, String[] options) {
            String[] opts = options;
            if (opts.length == 0) {
                opts = new String[] {
                    DEFAULT
                };
            }
            setModel(new StringPropertySelectionModel(opts));
            setMessages(messages);
            setResourcePrefix(LABEL);
        }
    }

    public void setRegion() {
        String region = getRegion();
        if (ModelWithDefaults.DEFAULT.equals(region)) {
            return;
        }
        int exitCode = getLocalizationContext().updateRegion(region);
        if (exitCode > 0) {
            recordSuccess("message.label.regionChanged");
        } else if (exitCode < 0) {
            recordFailure("message.label.regionFailed");
        }
    }

    public void setLanguage() {
        String language = getLanguage();
        if (ModelWithDefaults.DEFAULT.equals(language)) {
            return;
        }
        int exitCode = getLocalizationContext().updateLanguage(language);
        if (exitCode > 0) {
            recordSuccess("message.label.languageChanged");
        } else if (exitCode < 0) {
            recordFailure("message.label.lanuageFailed");
        }
    }

    public void uploadLocalizationPackage() {
        IUploadFile uploadFile = getUploadFile();
        if (uploadFile == null || StringUtils.isBlank(getUploadFile().getFilePath())) {
            recordFailure("message.noLocalizationFile");
            return;
        }
        try {
            String filePath = uploadFile.getFilePath();
            String fileName = AssetSelector.getSystemIndependentFileName(filePath);
            String extension = FilenameUtils.getExtension(fileName);
            // FIXME: only allow "tar" or "tgz" files - not sure why (how about .tar.gz?)
            if ("tar".equals(extension) || "tgz".equals(extension)) {
                LocalizationContext lc = getLocalizationContext();
                lc.installLocalizationPackage(uploadFile.getStream(), fileName);
                recordSuccess("message.installedOk");
            } else {
                recordFailure("message.invalidPackage");
            }
        } catch (UserException ex) {
            String msg = getMessages().getMessage(ex.getMessage());
            getValidator().record(new ValidatorException(msg));
        }
    }

    private void recordSuccess(String msgKey) {
        String msg = getMessages().getMessage(msgKey);
        TapestryUtils.recordSuccess(this, msg);
    }

    private void recordFailure(String msgKey) {
        String msg = getMessages().getMessage(msgKey);
        ValidatorException validatorException = new ValidatorException(msg);
        getValidator().record(validatorException);
    }
}
