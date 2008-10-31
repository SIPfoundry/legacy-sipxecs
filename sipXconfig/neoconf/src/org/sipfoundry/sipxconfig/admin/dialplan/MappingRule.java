/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Collections;
import java.util.Formatter;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;

/**
 * MappingRule
 *
 * Strangely, URLs in mapping rules are partially URL encoded
 *
 * <code>
 *
 *    Ampersand (&) 26
 *    Forward slash/Virgule (&quot;/&quot;) 2F
 *    Colon (:) 3A
 *    Semi-colon (;) 3B
 *    Equals (=) 3D
 *    Question mark (?) 3F
 *    'At' symbol (@) 40
 *
 *
 * </code>
 *
 * Common prefix
 *
 * sip:{0}@{mediaserver};play={voicemail}
 *
 * where 0 is {digits} or {vdigits}
 *
 * Autoattendant: /cgi-bin/voicemail/mediaserver.cgi?action=autoattendant&name={0}
 *
 * Voice mail: /cgi-bin/voicemail/mediaserver.cgi?action=retrieve&mailbox={digits}
 *
 * Voicemail fallback: /cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox={digits}
 *
 * Voicemail transfer: /cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox={vdigits}
 *
 */
public class MappingRule extends DialingRule {
    protected static final String PREFIX = "sip:%s@%s";
    protected static final String VMAIL_DEPOSIT = "deposit";
    protected static final String VMAIL_RETRIEVE = "retrieve";
    protected static final String AUTOATTENDANT = "autoattendant";
    private static final String FIELD_PARAM = "q=0.1";

    private String[] m_patterns;
    private String m_url;

    public MappingRule() {
        this(new String[] {}, StringUtils.EMPTY);
    }

    /**
     * @param patterns list of patterns
     * @param url
     */
    public MappingRule(String[] patterns, String url) {
        m_patterns = patterns;
        m_url = url;
        setEnabled(true);
    }

    @Override
    public String[] getPatterns() {
        return m_patterns;
    }

    @Override
    public Transform[] getTransforms() {
        UrlTransform transform = new UrlTransform();
        String urlParam = m_url;
        if (getSchedule() != null) {
            String validTime = getSchedule().calculateValidTime();
            String scheduleParam = String.format(VALID_TIME_PARAM, validTime);
            urlParam += ";" + scheduleParam;
        }
        transform.setUrl(urlParam);
        return new Transform[] {
            transform
        };
    }

    /**
     * Internal rule - added to mappingrules.xml
     */
    public boolean isInternal() {
        return true;
    }

    /**
     * Source permission rule - do not add permissions to authrules, add them to mapping rules.
     */
    @Override
    public boolean isTargetPermission() {
        return true;
    }

    public boolean isGatewayAware() {
        return false;
    }

    @Override
    public DialingRuleType getType() {
        return DialingRuleType.MAPPING_RULE;
    }

    public void setPatterns(String[] patterns) {
        m_patterns = patterns;
    }

    public void setUrl(String url) {
        m_url = url;
    }

    // specialized classes
    public static class Operator extends MappingRule {
        public Operator(AutoAttendant attendant, String extension, String[] aliases,
                MediaServer mediaServer) {
            this(attendant.getName(), attendant.getDescription(), attendant.getSystemName(),
                    extension, aliases, mediaServer);
        }

        public Operator(String name, String description, String systemName, String extension,
                String[] aliases, MediaServer mediaServer) {
            setName(name);
            setDescription(description);

            if (null == extension) {
                setPatterns(aliases);
            } else {
                setPatterns((String[]) ArrayUtils.add(aliases, 0, extension));
            }

            Map<String, String> additionalParams = new HashMap<String, String>();
            additionalParams.put("name", systemName);

            MediaServer.Operation operation = MediaServer.Operation.Autoattendant;
            String uriParams = mediaServer.getUriParameterStringForOperation(operation, null,
                    additionalParams);
            String headerParams = mediaServer.getHeaderParameterStringForOperation(operation,
                    null, additionalParams);
            setUrl(buildUrl('{' + CallDigits.FIXED_DIGITS.getName() + '}', mediaServer
                    .getHostname(), uriParams, headerParams, null));
        }
    }

    public static class VoicemailFallback extends MappingRule {
        private final MediaServer m_mediaServer;

        public VoicemailFallback(MediaServer mediaServer) {
            m_mediaServer = mediaServer;
            setPatterns(new String[] {
                "~~vm~."
            });
            MediaServer.Operation operation = MediaServer.Operation.VoicemailDeposit;
            setUrl(mediaServer.buildUrl(CallDigits.VARIABLE_DIGITS, operation,
                    MappingRule.FIELD_PARAM));
        }

        @Override
        public List<String> getPermissionNames() {
            return Collections.singletonList(m_mediaServer.getPermissionName().getName());
        }
    }

    public static class Voicemail extends MappingRule {
        public static final String VM_PREFIX = "~~vm~";

        public Voicemail(String voiceMail, MediaServer mediaServer) {
            setPatterns(new String[] {
                voiceMail
            });
            MediaServer.Operation operation = MediaServer.Operation.VoicemailRetrieve;
            setUrl(mediaServer.buildUrl(CallDigits.FIXED_DIGITS, operation, null));
        }
    }

    public static class VoicemailTransfer extends MappingRule {
        public VoicemailTransfer(String prefix, int extensionLen, MediaServer mediaServer) {
            DialPattern pattern = new DialPattern(prefix, extensionLen);
            setPatterns(new String[] {
                pattern.calculatePattern()
            });
            MediaServer.Operation operation = MediaServer.Operation.VoicemailDeposit;
            setUrl(mediaServer.buildUrl(CallDigits.VARIABLE_DIGITS, operation, null));
        }
    }

    /**
     * Builds a URL based on the provided digits, uri parameters, header parameters, and sip
     * parameters.
     *
     * @param digits - user digits (xxx@hostname)
     * @param hostname - hostname for the media server
     * @param uriParams - any uri parameters (can be null or empty string)
     * @param headerParams - any header parameters (can be null or empty string)
     * @param sipParams - any additional SIP params (can be null or empty string)
     * @return String representing the URL
     */
    static String buildUrl(String digitString, String hostname, String uriParams,
            String headerParams, String sipParams) {
        StringBuilder url = new StringBuilder("<");
        Formatter f = new Formatter(url);
        f.format(PREFIX, digitString, hostname);

        if (!StringUtils.isBlank(uriParams)) {
            url.append(';');
            url.append(uriParams);
        }

        if (!StringUtils.isBlank(headerParams)) {
            url.append('?');
            url.append(headerParams);
        }

        url.append('>');

        if (null != sipParams) {
            url.append(';');
            url.append(sipParams);
        }

        return url.toString();
    }
}
