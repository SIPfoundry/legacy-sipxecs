/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.conference;

public class NoSuchMemberException extends FreeswitchApiException {

    public NoSuchMemberException(ActiveConferenceMember member) {
        super("error.noSuchMember", member.getName());
    }

}
