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


/**
 * Names used by this enumeration have to corresponds to names used in resource.xml file in order
 * for replication to work properly.
 */
public enum ConfigFileType {
    MAPPING_RULES("mappingrules.xml.in"),
    FORWARDING_RULES("forwardingrules.xml.in"),
    FALLBACK_RULES("fallbackrules.xml.in"),
    AUTH_RULES("authrules.xml.in"),
    ORBITS("orbits.xml"),
    E911_RULES("e911rules.xml"),
    CONFERENCE_ADMINSION("conference_admission.xml"),
    CONFERENCE_CONFIGURATION("conference_configuration.xml"),
    ATTENDANT_SCHEDULE("attendant_schedule.xml"),
    ORGANIZATION_PREFS("organizationprefs.xml"),
    RESOURCE_LISTS("resource-lists.xml"),
    DOMAIN_CONFIG("domain-config"),
    PAGING_CONFIG("sipxpage.properties.in"),
    SBC_BRIDGE_CONFIG("sipxbridge.xml"),
    PROXY_CONFIG("sipXproxy-config"),
    REGISTRAR_CONFIG("registrar-config"),
    NAT_TRAVERSAL_RULES("nattraversalrules.xml");

    private String m_name;

    ConfigFileType(String name) {
        m_name = name;
    }

    public String getName() {
        return m_name;
    }
}
