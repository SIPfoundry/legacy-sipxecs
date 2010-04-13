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

import java.util.HashMap;
import java.util.Locale;
import java.util.Map;

import org.apache.hivemind.impl.AbstractMessages;
import org.sipfoundry.sipxconfig.site.common.LanguageSupport;

public class LocalizedLanguageMessages extends AbstractMessages {

    private Map<String, String> m_languageNameMap = new HashMap<String, String>();
    private LanguageSupport m_languageSupport;
    private Locale m_locale;

    public void setAvailableLanguages(String[] languages) {
        for (int i = 0; i < languages.length; i++) {
            m_languageNameMap.put(languages[i],
                    m_languageSupport.resolveLocaleName(languages[i]));
        }
    }

    public void setLanguageSupport(LanguageSupport languageSupport) {
        m_languageSupport = languageSupport;
    }

    public void setLocale(Locale locale) {
        m_locale = locale;
    }

    /**
     * Looks up the name of a language specified by the
     */
    protected String findMessage(String key) {
        String languageId = key;
        if (key.contains(".")) {
            languageId = key.substring(key.lastIndexOf('.') + 1);
        }
        return m_languageNameMap.get(languageId);
    }

    @Override
    protected Locale getLocale() {
        return m_locale;
    }

}
