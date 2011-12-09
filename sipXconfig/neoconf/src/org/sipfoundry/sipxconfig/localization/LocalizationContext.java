/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.localization;

import org.sipfoundry.sipxconfig.feature.GlobalFeature;

public interface LocalizationContext {
    static final GlobalFeature FEATURE = new GlobalFeature("localization");
    static final String DEFAULT = "default";
    static final String PROMPTS_DEFAULT = "stdprompts";

    public String getCurrentRegionId();

    public String getCurrentLanguage();

    public String getCurrentLanguageDir();

    public String[] getInstalledLanguages();

    public String[] getInstalledLanguageDirectories();

    public Localization getLocalization();

    public void updateRegion(String region);

    public int updateLanguage(String languageDirectory);
}
