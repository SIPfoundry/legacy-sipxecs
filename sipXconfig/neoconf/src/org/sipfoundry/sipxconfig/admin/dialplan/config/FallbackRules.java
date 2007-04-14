/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;

/**
 * Special type of falbackrules document with a single host match matching
 * standard SIPx hosts
 * 
 */
public class FallbackRules extends MappingRules {
    public void generate(IDialingRule rule) {
        if (!rule.isInternal()) {
            generateRule(rule);
        }
    }
    
    public ConfigFileType getType() {
        return ConfigFileType.FALLBACK_RULES;
    }
}
