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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 * Parses the string returned from a FreeSWITCH XML-RPC call.
 */
public class FreeswitchApiResultParserImpl implements FreeswitchApiResultParser {
    private static final Log LOG = LogFactory.getLog(FreeswitchApiResultParser.class);

    private static final String EMPTY_STRING = "No active conferences.";

    private static final String MEMBER_DELIMITER = ";";

    private static final Pattern CONFERENCE_NOT_FOUND_PATTERN = Pattern.compile("Conference (.+) not found\\n");

    private static final Pattern CONFERENCE_NAME_PATTERN = Pattern
            .compile("Conference ([\\w ]+) \\((\\d+) members?( locked)?\\)");

    // the misspelling is intentional - typo in freeswitch API
    private static final Pattern INVALID_MEMBER_PATTERN = Pattern.compile("Non-Exist[ae]nt ID [\\d]+\\n");

    /**
     * Verifies that a member action was completed successfully and that the target member exists.
     * 
     * @throws NoSuchMemberException if the member does not exist
     */
    public boolean verifyMemberAction(String resultString, ActiveConferenceMember member) {
        if (INVALID_MEMBER_PATTERN.matcher(resultString).matches()) {
            throw new NoSuchMemberException(member);
        }
        return true;
    }

    /**
     * Verifies that a conference action was completed successfully and that the target conference
     * exists.
     * 
     * @throws NoSuchConferenceException if the member does not exist
     */
    public boolean verifyConferenceAction(String resultString, Conference conference) {
        if (CONFERENCE_NOT_FOUND_PATTERN.matcher(resultString).matches()) {
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
        if (resultString.equals(EMPTY_STRING)) {
            return 0;
        }
        Matcher matcher = CONFERENCE_NAME_PATTERN.matcher(resultString);
        boolean found = matcher.find();
        int count = 0;
        while (found) {
            count++;
            found = matcher.find(matcher.end());
        }

        return count;
    }

    /**
     * Parses a result string and creates a list of ActiveConference objects representing each
     * conference.
     * 
     * @param resultString the string returned from the FreeSWITCH API call
     * @return a List of ActiveConference objects
     */
    public List<ActiveConference> getActiveConferences(String resultString) {
        List<ActiveConference> activeConferences = new ArrayList<ActiveConference>();

        if (!resultString.equals(EMPTY_STRING)) {
            Matcher matcher = CONFERENCE_NAME_PATTERN.matcher(resultString);
            boolean found = matcher.find();
            while (found) {
                String conferenceName = matcher.group(1);
                int members = Integer.parseInt(matcher.group(2));
                boolean locked = (matcher.group(3) != null);
                activeConferences.add(new ActiveConference(conferenceName, members, locked));
                found = matcher.find(matcher.end());
            }
        }

        return activeConferences;
    }

    public List<ActiveConferenceMember> getConferenceMembers(String resultString) {
        List<ActiveConferenceMember> members = new ArrayList<ActiveConferenceMember>();

        try {
            BufferedReader lineReader = new BufferedReader(new StringReader(resultString));
            String currentLine = "";
            while ((currentLine = lineReader.readLine()) != null) {
                String[] fields = currentLine.split(MEMBER_DELIMITER);
                int fieldIndex = 0;

                ActiveConferenceMember member = new ActiveConferenceMember();
                member.setId(Integer.parseInt(fields[fieldIndex++]));

                String sipAddress = fields[fieldIndex++].split("/")[2];
                member.setUuid(fields[fieldIndex++]);
                String callerIdName = fields[fieldIndex++];
                fieldIndex++; // skip caller ID number

                String permissions = fields[fieldIndex++];
                member.setCanHear(permissions.contains("hear"));
                member.setCanSpeak(permissions.contains("speak"));

                member.setName(callerIdName + " (" + sipAddress + ")");

                member.setVolumeIn(Integer.parseInt(fields[fieldIndex++]));
                member.setVolumeOut(Integer.parseInt(fields[fieldIndex++]));
                member.setEnergyLevel(Integer.parseInt(fields[fieldIndex++]));

                members.add(member);
            }
        } catch (IOException ioe) {
            // This should never happen. A StringReader shouldn't have an IOException
            LOG.error("Error reading line from StringReader");
        }

        return members;
    }
}
