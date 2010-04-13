/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.callcontroller;

import javax.sip.Dialog;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;



/**
 * Sip utility service functions complete w/server current configuration
 */
public interface SipService {

    /**
     * 
     * @param credentials - creds of the user to impersonate.
     * @param sourceAddrSpec - third party call controller sourceAddr
     * @param displayName - displayName for the calling party.
     * @param destinationAddSpec - third party call controller destAddr
     * @param subject - subject.
     * @param inviteForwardingAllowed - whether or not INVITE forwarding is allowd.
     * @param dialogContext - the dialog context to attach to the newly created dialog.
     */
    public Dialog sendRefer(UserCredentialHash credentials, String sourceAddrSpec, String displayName, String destinationAddrSpec, 
            String referTarget, String subject, boolean inviteForwardingAllowed,
            DialogContext dialogContext, int timeout);
}
