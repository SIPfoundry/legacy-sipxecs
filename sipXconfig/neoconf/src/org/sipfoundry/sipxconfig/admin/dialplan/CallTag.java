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
    AA("Auto Attendant"), // auto-attendant
    CUST("Custom"), // custom
    EMERG("Emergency"), // emergency
    INTN("International"), // international
    LD("Long Distance"), // long distance
    LOCL("Local"), // local
    MOH("Music on Hold"), // music on hold
    PAGE("Page"), // paging
    RL("Resource List"), // resource list server
    REST("Restricted"), // restricted dialing
    STS("Site-To-Site"), // site to site
    MOB("Mobile"), // mobile
    TF("Toll Free"), // toll free
    VM("VoiceMail"), // voicemail
    VMR("VoiceMail Redirect"), // voicemail redirect
    AL("Alias"),  // Alias
    INT("Internal"), // Internal
    PARK("Park"), // Park
    RPARK("Retrieve Park"), // Retrieve Park
    DPUP("Directed Pickup"), // Directed Pickup
    UNK("Unknown"); // unknown

    private String m_name;

    CallTag(String name) {
        m_name = name;
    }

    public String getName() {
        return m_name;
    }
}
