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

import org.apache.commons.lang.enums.Enum;

/**
 * Names used by this enumeration have to corresponds to names used in resource.xml file in order
 * for replication to work properly.
 */
public final class ConfigFileType extends Enum {
    public static final ConfigFileType MAPPING_RULES = new ConfigFileType("mappingrules.xml");
    public static final ConfigFileType FORWARDING_RULES = new ConfigFileType("forwardingrules.xml");
    public static final ConfigFileType FALLBACK_RULES = new ConfigFileType("fallbackrules.xml");
    public static final ConfigFileType AUTH_RULES = new ConfigFileType("authrules.xml");
    public static final ConfigFileType ORBITS = new ConfigFileType("orbits.xml");
    public static final ConfigFileType E911_RULES = new ConfigFileType("e911rules.xml");
    public static final ConfigFileType CONFERENCE_ADMINSION = new ConfigFileType("conference_admission.xml");
    public static final ConfigFileType CONFERENCE_CONFIGURATION = new ConfigFileType("conference_configuration.xml");
    public static final ConfigFileType ATTENDANT_SCHEDULE = new ConfigFileType("attendant_schedule.xml");
    public static final ConfigFileType ORGANIZATION_PREFS = new ConfigFileType("organizationprefs.xml");
    public static final ConfigFileType RESOURCE_LISTS = new ConfigFileType("resource-lists.xml");
    public static final ConfigFileType DOMAIN_CONFIG = new ConfigFileType("domain-config");
    public static final ConfigFileType PAGING_CONFIG = new ConfigFileType("sipxpage.properties.in");
    public static final ConfigFileType SBC_BRIDGE_CONFIG = new ConfigFileType("sipxbridge.xml");
    public static final ConfigFileType PROXY_CONFIG = new ConfigFileType("sipXproxy-config");
    public static final ConfigFileType REGISTRAR_CONFIG = new ConfigFileType("registrar-config");
    public static final ConfigFileType NAT_TRAVERSAL_RULES = new ConfigFileType("nattraversalrules.xml");

    private ConfigFileType(String name) {
        super(name);
    }
}
