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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.NoSuchElementException;
import java.util.Scanner;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.setting.type.SipUriSetting;

/**
 * Parses the string returned from a FreeSWITCH XML-RPC call.
 */
public class FreeswitchApiResultParserImpl implements FreeswitchApiResultParser {
    private static final Log LOG = LogFactory.getLog(FreeswitchApiResultParser.class);

    private static final String EMPTY_STRING = "No active conferences.";

    private static final Pattern CONFERENCE_NOT_FOUND_PATTERN = Pattern.compile("Conference (.+) not found\\n");

    private static final Pattern CONFERENCE_NAME_PATTERN = Pattern
            .compile("Conference ([" + SipUriSetting.USER_NAME + "]+) \\((\\d+) members? rate: \\d+( locked)?\\)");

    private static final int CONFERENCE_NAME_PATTERN_GROUP_INDEX = 1;
    private static final int CONFERENCE_MEMBERS_PATTERN_GROUP_INDEX = 2;
    private static final int CONFERENCE_LOCKED_PATTERN_GROUP_INDEX = 3;

    // the misspelling is intentional - typo in freeswitch API
    private static final Pattern INVALID_MEMBER_PATTERN = Pattern.compile("Non-Exist[ae]nt ID [\\d]+\\n");

    private static final String COMMAND_LIST_DELIM = ">,<";

    /**
     * Verifies that a member action was completed successfully and that the target member exists.
     *
     * @throws NoSuchMemberException if the member does not exist
     */
    public boolean verifyMemberAction(String resultString, ActiveConferenceMember member) {
        if (StringUtils.isBlank(resultString)) {
            throw new NoSuchMemberException(member);
        }
        if (INVALID_MEMBER_PATTERN.matcher(resultString).matches()) {
            throw new NoSuchMemberException(member);
        }
        return true;
    }

    /**
     * Verifies that a conference action was completed successfully and that the target conference
     * exists.
     *
     * @throws NoSuchConferenceException if the conference does not exist
     */
    public boolean verifyConferenceAction(String resultString, Conference conference) {
        if (StringUtils.isBlank(resultString)) {
            throw new NoSuchConferenceException(conference);
        }
        if (CONFERENCE_NOT_FOUND_PATTERN.matcher(resultString).matches()) {
            new NoSuchConferenceException(conference).printStackTrace();
            throw new NoSuchConferenceException(conference);
        }
        return true;
    }

    /**
     * Parses a result string and determines the total number of active conferences.
     *
     * @param resultString the string returned from the FreeSWITCH API call
     * @return the number of active conferences reported by FreeSWITCH.
     */
    public int getActiveConferenceCount(String resultString) {
        if (StringUtils.isBlank(resultString)) {
            return 0;
        }
        if (resultString.equals(EMPTY_STRING)) {
            return 0;
        }
        int count = 0;
        Matcher matcher = CONFERENCE_NAME_PATTERN.matcher(resultString);
        while (matcher.find()) {
            count++;
        }

        return count;
    }

    /**
     * Parses a result string and determines if the conference is locked.
     *
     * @param resultString the string returned from the FreeSWITCH API call
     * @return whether or not the conference is locked.
     */
    public boolean isConferenceLocked(String resultString) {
        Matcher matcher = CONFERENCE_NAME_PATTERN.matcher(resultString);
        boolean isLocked = false;

        if (matcher.find()) {
            isLocked = (matcher.group(CONFERENCE_LOCKED_PATTERN_GROUP_INDEX) != null);
        }

        return isLocked;
    }

    /**
     * Parses a result string and creates a list of ActiveConference objects representing each
     * conference.
     *
     * @param resultString the string returned from the FreeSWITCH API call
     * @return a List of ActiveConference objects
     */
    public List<ActiveConference> getActiveConferences(String resultString) {
        if (StringUtils.isBlank(resultString)) {
            return Collections.emptyList();
        }
        if (resultString.equals(EMPTY_STRING)) {
            return Collections.emptyList();
        }
        List<ActiveConference> activeConferences = new ArrayList<ActiveConference>();

        Matcher matcher = CONFERENCE_NAME_PATTERN.matcher(resultString);
        while (matcher.find()) {
            String conferenceName = matcher.group(CONFERENCE_NAME_PATTERN_GROUP_INDEX);
            int members = Integer.parseInt(matcher.group(CONFERENCE_MEMBERS_PATTERN_GROUP_INDEX));
            boolean locked = (matcher.group(CONFERENCE_LOCKED_PATTERN_GROUP_INDEX) != null);
            activeConferences.add(new ActiveConference(conferenceName, members, locked));
        }

        return activeConferences;
    }

    public List<ActiveConferenceMember> getConferenceMembers(String resultString, Conference conference) {
        List<ActiveConferenceMember> members = new ArrayList<ActiveConferenceMember>();

        String conferenceName = conference.getName();
        Scanner scanner = new Scanner(resultString);
        while (scanner.hasNextLine()) {
            String line = scanner.nextLine();
            try {
                ActiveConferenceMember member = parseConferenceMember(line, conferenceName);
                members.add(member);
            } catch (NoSuchElementException e) {
                LOG.error("Skipping conference line:" + line);
            }
        }

        return members;
    }

    private ActiveConferenceMember parseConferenceMember(String line, String conferenceName) {
        ActiveConferenceMember member = new ActiveConferenceMember();

        Scanner scan = new Scanner(line);
        scan.useDelimiter(COMMAND_LIST_DELIM);

        member.setId(scan.nextInt());

        String sipAddress = scan.next().split("/")[2];

        member.setUuid(scan.next());

        String callerIdName = scan.next();
        if (callerIdName.equals(conferenceName)) {
            callerIdName = "";
        }

        scan.next(); // skip caller ID number

        String permissions = scan.next();
        member.setCanHear(permissions.contains("hear"));
        member.setCanSpeak(permissions.contains("speak"));

        member.setName(callerIdName + " (" + sipAddress + ")");

        member.setVolumeIn(scan.nextInt());
        member.setVolumeOut(scan.nextInt());
        member.setEnergyLevel(scan.nextInt());
        return member;
    }

}
