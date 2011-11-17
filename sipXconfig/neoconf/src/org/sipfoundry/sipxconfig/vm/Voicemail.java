/*
 *
 *
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.Serializable;
import java.util.Date;

public interface Voicemail extends Serializable {

    String getFolderId();

    String getUserId();

    String getMessageId();

    boolean isHeard();

    String getSubject();

    String getForwardedSubject();

    String getFromBrief();

    String getFrom();

    String getForwardedFromBrief();

    Date getTimestamp();

    Date getForwardedTimestamp();

    int getDurationsecs();

    int getForwardedDurationsecs();

    boolean isForwarded();

}
