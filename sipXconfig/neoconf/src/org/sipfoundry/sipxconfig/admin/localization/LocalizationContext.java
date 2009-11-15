/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.localization;

import java.io.InputStream;

public interface LocalizationContext {
    static final String DEFAULT = "default";
    static final String PROMPTS_DEFAULT = "stdprompts";

    public String getCurrentRegionId();

    public String getCurrentLanguage();

    public String getCurrentLanguageDir();

    public String[] getInstalledRegions();

    public String[] getInstalledLanguages();

    public String[] getInstalledLanguageDirectories();

    public Localization getLocalization();

    public int updateRegion(String region);

    public int updateLanguage(String languageDirectory);

    public void installLocalizationPackage(InputStream stream, String name);
}
