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
    AA, // auto-attendant
    CUST, // custom
    EMERG, // emergency
    INTN, // international
    LD, // long distance
    LOCL, // local
    MOH, // music on hold
    PAGE, // paging
    RL, // resource list server
    REST, // restricted dialing
    STS, // site to site
    MOB, // mobile
    TF, // toll free
    VM, // voicemail
    VMR, // voicemail redirect
    UNK; // unknown
}
