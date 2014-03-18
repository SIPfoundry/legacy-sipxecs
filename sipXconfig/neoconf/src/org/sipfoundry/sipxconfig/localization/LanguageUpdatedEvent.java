/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.localization;

import org.springframework.context.ApplicationEvent;

@SuppressWarnings("serial")
public class LanguageUpdatedEvent extends ApplicationEvent {
    private String m_promptsDir;
    private String m_currentLanguageDir;

    public LanguageUpdatedEvent(Object source, String promptsDir, String currentLanguageDir) {
        super(source);
        setPromptsDir(promptsDir);
        setCurrentLanguageDir(currentLanguageDir);
    }

    public String getPromptsDir() {
        return m_promptsDir;
    }

    public void setPromptsDir(String promptsDir) {
        m_promptsDir = promptsDir;
    }

    public String getCurrentLanguageDir() {
        return m_currentLanguageDir;
    }

    public void setCurrentLanguageDir(String currentLanguageDir) {
        m_currentLanguageDir = currentLanguageDir;
    }
}
