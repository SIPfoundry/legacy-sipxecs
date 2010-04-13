/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.components.CompositeMessages;
import org.sipfoundry.sipxconfig.components.ExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.site.admin.LocalizedExtraOptionModelDecorator;
import org.sipfoundry.sipxconfig.site.admin.LocalizedLanguageMessages;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class InstalledLanguages extends BaseComponent {

    @Parameter
    public abstract String getLanguage();

    @Parameter
    public abstract String getLanguageDescription();

    @Parameter(defaultValue = "ognl:LocalizationContext.DEFAULT")
    public abstract String getDefaultLanguage();

    public abstract ExtraOptionModelDecorator getLanguagesSelectionModel();
    public abstract void setLanguagesSelectionModel(ExtraOptionModelDecorator languagesModel);

    @InjectObject("spring:localizationContext")
    public abstract LocalizationContext getLocalizationContext();
    @InjectObject("spring:localizedLanguageMessages")
    public abstract LocalizedLanguageMessages getLocalizedLanguageMessages();

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        if (getLanguagesSelectionModel() == null) {
            initLanguages();
        }
    }

    public void initLanguages() {
        String[] availableLanguages = getLocalizationContext().getInstalledLanguages();
        LocalizedLanguageMessages languageMessages = getLocalizedLanguageMessages();
        languageMessages.setAvailableLanguages(availableLanguages);
        CompositeMessages messages = new CompositeMessages(languageMessages, getMessages());
        ExtraOptionModelDecorator decoratedModel = new LocalizedExtraOptionModelDecorator(messages, availableLanguages);
        decoratedModel.setExtraLabel(getMessages().getMessage("label.default"));
        decoratedModel.setExtraOption(getDefaultLanguage());
        setLanguagesSelectionModel(decoratedModel);
    }
}
