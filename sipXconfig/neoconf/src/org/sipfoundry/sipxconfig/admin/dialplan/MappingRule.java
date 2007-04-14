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

import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.Collections;
import java.util.Formatter;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Transform;
import org.sipfoundry.sipxconfig.admin.dialplan.config.UrlTransform;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * MappingRule
 * 
 * Strangely URLs im mapping rules are partially URL encoded
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
    protected static final String PREFIX = "sip:{%s}@{mediaserver};voicexml={voicemail}";
    protected static final String SUFFIX = "/cgi-bin/voicemail/mediaserver.cgi?action=";
    protected static final String VMAIL_DEPOSIT = "deposit";
    protected static final String VMAIL_RETRIEVE = "retrieve";
    protected static final String AUTOATTENDANT = "autoattendant";

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

    public String[] getPatterns() {
        return m_patterns;
    }

    public Transform[] getTransforms() {
        UrlTransform transform = new UrlTransform();
        transform.setUrl(m_url);
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
        public Operator(AutoAttendant attendant, String extension, String[] aliases) {
            this(attendant.getName(), attendant.getDescription(), attendant.getSystemName(),
                    extension, aliases);
        }

        public Operator(String name, String description, String systemName, String extension,
                String[] aliases) {
            setName(name);
            setDescription(description);

            if (null == extension) {
                setPatterns(aliases);
            } else {
                setPatterns((String[]) ArrayUtils.add(aliases, 0, extension));
            }

            Map params = new HashMap();
            params.put("name", systemName);

            setUrl(buildUrl(CallDigits.FIXED_DIGITS, AUTOATTENDANT, params));
        }
    }

    public static class VoicemailFallback extends MappingRule {
        public VoicemailFallback(int extensionLen) {
            DialPattern pattern = new DialPattern(StringUtils.EMPTY, extensionLen);
            setPatterns(new String[] {
                pattern.calculatePattern()
            });
            setUrl(buildUrl(CallDigits.FIXED_DIGITS, MappingRule.VMAIL_DEPOSIT,
                    getMailboxParams(CallDigits.FIXED_DIGITS), ";q=0.1"));
        }

        public List<String> getPermissionNames() {
            return Collections.singletonList(PermissionName.VOICEMAIL.getName());
        }
    }

    public static class Voicemail extends MappingRule {
        public Voicemail(String voiceMail) {
            setPatterns(new String[] {
                voiceMail
            });
            setUrl(buildUrl(CallDigits.FIXED_DIGITS, MappingRule.VMAIL_RETRIEVE, null));
        }

    }

    public static class VoicemailTransfer extends MappingRule {
        public VoicemailTransfer(String prefix, int extensionLen) {
            DialPattern pattern = new DialPattern(prefix, extensionLen);
            setPatterns(new String[] {
                pattern.calculatePattern()
            });
            setUrl(buildUrl(CallDigits.VARIABLE_DIGITS, MappingRule.VMAIL_DEPOSIT,
                    getMailboxParams(CallDigits.VARIABLE_DIGITS)));
        }
    }

    static String buildUrl(CallDigits digits, String action, Map<String, String> params) {
        return buildUrl(digits, action, params, null);
    }

    /**
     * Creates URL for mediaserver
     * 
     * @param digits - vdigits or digits
     * @param action - autoattendant, sos, deposit or retrieve
     * @param params - any additional mediaserver params (can be null or empty)
     * @param sipParams - any additional SIP params (can be null or empty)
     * @return String representign the URL
     */
    static String buildUrl(CallDigits digits, String action, Map<String, String> params,
            String sipParams) {
        try {
            StringBuilder url = new StringBuilder("<");
            String suffix = URLEncoder.encode(SUFFIX, "UTF-8");
            Formatter f = new Formatter(url);
            f.format(PREFIX, digits.getName());
            url.append(suffix);
            url.append(action);
            if (null != params) {
                for (Map.Entry<String, String> entry : params.entrySet()) {
                    url.append("%26"); // & - URL encoded
                    url.append(entry.getKey());
                    url.append("%3D"); // = - URL encoded
                    url.append(entry.getValue());
                }
            }
            url.append('>');
            if (null != sipParams) {
                url.append(sipParams);
            }
            return url.toString();
        } catch (UnsupportedEncodingException e) {
            throw new RuntimeException(e);
        }
    }

    protected Map<String, String> getMailboxParams(CallDigits digits) {
        Map<String, String> map = new HashMap<String, String>();
        map.put("mailbox", String.format("{%s}", digits.getEscapedName()));
        return map;
    }
}
