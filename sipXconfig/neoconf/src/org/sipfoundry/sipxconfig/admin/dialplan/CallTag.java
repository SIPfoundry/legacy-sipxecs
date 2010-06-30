/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

/**
 * CallTag is used by call resolver to classify various calls based on what type of dialing rules
 * has been used to process the call.
 */
public enum CallTag {
    AA("Auto Attendant", "AA"), // auto-attendant
    CUST("Custom", "CUSTM"), // custom
    EMERG("Emergency", "EMER"), // emergency
    INTN("International", "INTN"), // international
    LD("Long Distance", "LD"), // long distance
    LOCL("Local", "LCL"), // local
    MOH("Music on Hold", "MOH"), // music on hold
    PAGE("Page", "PAGE"), // paging
    RL("Resource List", "RL"), // resource list server
    REST("Restricted", "REST"), // restricted dialing
    STS("Site-To-Site", "STS"), // site to site
    MOB("Mobile", "MOB"), // mobile
    TF("Toll Free", "TF"), // toll free
    VM("VoiceMail", "VMAIL"), // voicemail
    VMR("VoiceMail Redirect", "RVMAIL"), // voicemail redirect
    AL("Alias", "AL"),  // Alias
    INT("Internal", "INT"), // Internal
    PARK("Park", "PARK"), // Park
    RPARK("Retrieve Park", "RPARK"), // Retrieve Park
    DPUP("Directed Pickup", "DPUP"), // Directed Pickup
    AUTH("Authorization Code", "AUTH"), // Authorization Code
    UNK("Unknown", "UNK"), // unknown
    FAX("Fax", "FAX"); //Fax extension

    private String m_name;
    private String m_shortname;

    CallTag(String name, String sname) {
        m_name = name;
        m_shortname = sname;
    }

    public String getName() {
        return m_name;
    }

    public String getShortName() {
        return m_shortname;
    }
}
