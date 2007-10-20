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
    String getCurrentRegionId();

    String getCurrentLanguageId();

    String[] getInstalledRegions();

    String[] getInstalledLanguages();

    Localization getLocalization();

    int updateRegion(String region);

    int updateLanguage(String language);

    void installLocalizationPackage(InputStream stream, String name);
}
