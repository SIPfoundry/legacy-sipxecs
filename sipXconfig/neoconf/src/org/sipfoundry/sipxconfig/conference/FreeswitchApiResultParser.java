/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.conference;

import java.util.List;

/**
 * When calling XML-RPC methods on the FreeSWITCH API, the results are
 * returned as one monolithic String. This class parses those Strings
 * into usable result objects.
 */
public interface FreeswitchApiResultParser {

    public boolean verifyMemberAction(String resultString, ActiveConferenceMember member);

    public boolean verifyConferenceAction(String resultString, Conference conference);

    /**
     * Parses a result string and determines the total number of
     * active conferences.
     *
     * @param resultString the string returned from the FreeSWITCH API call
     * @return the number of active conferences reported by FreeSWITCH.
     */
    public int getActiveConferenceCount(String resultString);

    /**
     * Parses a result string and determines if the conference is locked.
     *
     * @param resultString the string returned from the FreeSWITCH API call
     * @return whether or not the conference is locked.
     */
    public boolean isConferenceLocked(String resultString);

    /**
     * Parses a result string and creates a list of ActiveConference
     * objects representing each conference.
     *
     * @param resultString the string returned from the FreeSWITCH API call
     * @return a List of ActiveConference objects
     */
    public List<ActiveConference> getActiveConferences(String resultString);

    public List<ActiveConferenceMember> getConferenceMembers(String resultString, Conference conference);
}
