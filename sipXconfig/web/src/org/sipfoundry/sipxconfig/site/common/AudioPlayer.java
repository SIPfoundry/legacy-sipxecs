/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.common;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.Parameter;

@ComponentClass
public abstract class AudioPlayer extends BaseComponent {

    @Parameter(required = true)
    public abstract String getUrl();
    @Parameter
    public abstract String getPlayerId();
    @Parameter(defaultValue = "false")
    public abstract boolean getAutoplay();

    public abstract String getActivePlayerId();
    public abstract void setActivePlayerId(String activePlayerId);

    @Asset("/images/play.png")
    public abstract IAsset getPlayVoicemailAsset();

    public void notifyPlay() {
        setActivePlayerId(getPlayerId());
    }

    public boolean getShowPlayer() {
        return (getPlayerId() == null || (getActivePlayerId() != null
                && StringUtils.equals(getActivePlayerId(), getPlayerId())));
    }

}
