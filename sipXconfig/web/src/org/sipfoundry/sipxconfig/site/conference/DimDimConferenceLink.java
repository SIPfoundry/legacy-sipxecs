/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.conference;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.conference.DimDimConference;


@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class DimDimConferenceLink extends BaseComponent {

    @Parameter(required = true)
    public abstract DimDimConference getDimDimConference();

    public String getStartWebConference() {
        return getMessages().getMessage("button.startWebConference");
    }

    public String getStartUrl() {
        return "http://" + getDimDimConference().getDimDimHost() + "/mashup/start";
    }

    public String getInviteEmail() {
        return String.format("mailto:?subject=%s&body=%s.",
                getInviteEmailSubject(), getInviteEmailBody());
    }

    private String getInviteEmailSubject() {
        return getMessages().format("invite.email.subject",
                getMessages().getMessage("conferenceVendor"));
    }

    private String getInviteEmailBody() {
        return getMessages().format("invite.email.body",
                new Object[] {
                    getDimDimConference().getDid(),
                    getDimDimConference().getDimDimHost(),
                    getDimDimConference().getUser(),
                    getDimDimConference().getAttendeePasscode()
                }
        );
    }
}
