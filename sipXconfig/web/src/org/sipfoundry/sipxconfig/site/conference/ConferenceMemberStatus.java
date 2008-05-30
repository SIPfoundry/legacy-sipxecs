/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.conference;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.conference.ActiveConferenceMember;

public abstract class ConferenceMemberStatus extends BaseComponent {

    @Parameter(required = true)
    public abstract void setConferenceMember(ActiveConferenceMember member);
    public abstract ActiveConferenceMember getConferenceMember();

    @Asset("/images/sound.png")
    public abstract IAsset getSoundIcon();

    @Asset("/images/deaf.png")
    public abstract IAsset getDeafIcon();

    @Asset("/images/talk.png")
    public abstract IAsset getTalkIcon();

    @Asset("/images/no-talk.png")
    public abstract IAsset getMuteIcon();

}
