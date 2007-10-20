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

import java.util.StringTokenizer;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.LocalizedOptionModelDecorator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class LocalizationPage extends UserBasePage implements PageBeginRenderListener {
    private static final String LABEL = "label.";

    private static final String DEFAULT = "default";

    @InjectObject(value = "spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();

    public abstract LocalizedOptionModelDecorator getRegionList();

    public abstract void setRegionList(LocalizedOptionModelDecorator regionList);

    public abstract LocalizedOptionModelDecorator getLanguageList();

    public abstract void setLanguageList(LocalizedOptionModelDecorator languageList);

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
        LocalizedOptionModelDecorator optionModel = new LocalizedOptionModelDecorator();
        String[] regions = getLocalizationContext().getInstalledRegions();
        if (regions.length == 0) {
            // No regions found - display "default"
            regions = new String[1];
            regions[0] = DEFAULT;
        }
        optionModel.setModel(new StringPropertySelectionModel(regions));
        optionModel.setMessages(getMessages());
        optionModel.setResourcePrefix(LABEL);
        setRegionList(optionModel);
    }

    private void initLanguages() {
        LocalizedOptionModelDecorator optionModel = new LocalizedOptionModelDecorator();
        String[] languages = getLocalizationContext().getInstalledLanguages();
        if (languages.length == 0) {
            // No languages found - display "default"
            languages = new String[1];
            languages[0] = DEFAULT;
        }
        optionModel.setModel(new StringPropertySelectionModel(languages));
        optionModel.setMessages(getMessages());
        optionModel.setResourcePrefix(LABEL);
        setLanguageList(optionModel);
    }

    public void setRegion() {
        int exitCode = getLocalizationContext().updateRegion(getRegion());
        if (exitCode > 0) {
            TapestryUtils.recordSuccess(this, getMessages().getMessage("message.label.regionChanged"));
        } else if (exitCode < 0) {
            getValidator().record(new ValidatorException(getMessages().getMessage("message.label.regionFailed")));
        }
    }

    public void setLanguage() {
        int exitCode = getLocalizationContext().updateLanguage(getLanguage());
        if (exitCode > 0) {
            TapestryUtils.recordSuccess(this, getMessages().getMessage("message.label.languageChanged"));
        } else if (exitCode < 0) {
            getValidator().record(new ValidatorException(getMessages().getMessage("message.label.languageFailed")));
        }
    }

    public void uploadLocalizationPackage() {
        IUploadFile uploadFile = getUploadFile();
        if ((uploadFile == null) 
            || (uploadFile.getFilePath() == null)
            || (uploadFile.getFilePath().length() == 0)) {
            getValidator().record(new ValidatorException(getMessages().getMessage("message.noLocalizationFile")));
            return;
        }
        try {
            String fileName = null;
            String extension = null;
            StringTokenizer st = new StringTokenizer(uploadFile.getFileName(), "\\");
            while (st.hasMoreTokens()) {
                fileName = st.nextToken();
            }
            if (fileName != null) {
                int extensionIndex = fileName.lastIndexOf('.');
                if (extensionIndex > 0) {
                    extension = fileName.substring(extensionIndex);
                }
            }
            if ((extension == null)
                || ((extension.compareTo(".tar") != 0)
                    && (extension.compareTo(".tgz") != 0))) {
                getValidator().record(new ValidatorException(getMessages().getMessage("message.invalidPackage")));
            } else {
                if (getLocalizationContext().installLocalizationPackage(uploadFile.getStream(), fileName) == 0) {
                    TapestryUtils.recordSuccess(this, getMessages().getMessage("message.installedOk"));
                } else {
                    getValidator().record(new ValidatorException(getMessages().getMessage("message.installError")));
                }
            }
        } catch (UserException ex) {
            getValidator().record(new ValidatorException(getMessages().getMessage(ex.getMessage())));
        }
    }
}
