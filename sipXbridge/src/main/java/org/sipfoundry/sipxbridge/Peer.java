/*
 *  Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 *  Contributors retain copyright to elements licensed under a Contributor Agreement.
 *  Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxbridge;

public class Peer {

    private String mTrustedDomain;
    private String mInternalUser;

    public void setTrustedDomain(String trustedDomain) {
        this.mTrustedDomain = trustedDomain;
    }

    public String getTrustedDomain() {
        return mTrustedDomain;
    }

    public void setInternalUser(String internalUser) {
        this.mInternalUser = internalUser;
    }

    public String getInternalUser() {
        return mInternalUser;
    }
}
