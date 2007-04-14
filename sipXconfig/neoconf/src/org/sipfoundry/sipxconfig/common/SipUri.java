/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;

public final class SipUri {
    public static final String SIP_PREFIX = "sip:";
    public static final int DEFAULT_SIP_PORT = 5060;

    private static final Pattern EXTRACT_USER_RE = Pattern.compile("\\s*<?(?:sip:)?(.+?)@.+");
    private static final Pattern EXTRACT_FULL_USER_RE = Pattern
            .compile("(?:\"(.*)\")?\\s*<?(?:sip:)?(.+?)@.+");
    private static final Pattern SIP_URI_RE = Pattern
            .compile("(?:\".*\")?\\s*<?(?:sip:)?.+?@.+>?");

    private SipUri() {
        // utility class
    }

    public static String format(User user, String domainName) {
        return format(user.getDisplayName(), user.getUserName(), domainName);
    }

    public static String format(String displayName, String userName, String domainName) {
        StringBuilder uri = new StringBuilder();
        boolean needsWrapping = StringUtils.isNotBlank(displayName);

        if (needsWrapping) {
            uri.append('"');
            uri.append(displayName);
            uri.append('"');
        }

        String uriProper = format(userName, domainName, needsWrapping);
        uri.append(uriProper);
        return uri.toString();
    }

    public static String format(String userName, String domainName, int port) {
        String uri = String.format("sip:%s@%s:%d", userName, domainName, port);
        return uri;
    }

    public static String format(String domainName, int port) {
        String uri = String.format("sip:%s:%d", domainName, port);
        return uri;
    }

    public static int parsePort(String sPort, int defaultPort) {
        if (StringUtils.isBlank(sPort)) {
            return defaultPort;
        }
        int port = Integer.parseInt(sPort);
        return port;
    }

    public static String formatIgnoreDefaultPort(String userName, String domain, int port) {
        if (port == DEFAULT_SIP_PORT) {
            return format(userName, domain, false);
        }
        return format(userName, domain, port);
    }

    public static String formatIgnoreDefaultPort(String displayName, String userName,
            String domain, int port) {
        String baseUri = formatIgnoreDefaultPort(userName, domain, port);
        if (displayName == null) {
            return baseUri;
        }
        String uri = String.format("\"%s\"<%s>", displayName, baseUri);
        return uri;
    }

    public static String format(String userName, String domain, boolean quote) {
        String format = quote ? "<sip:%s@%s>" : "sip:%s@%s";
        return String.format(format, userName, domain);
    }

    public static String normalize(String uri) {
        String result = uri.trim();
        if (result.startsWith(SIP_PREFIX)) {
            return result;
        }
        return SIP_PREFIX + result;
    }

    public static String extractUser(String uri) {
        if (uri == null) {
            return null;
        }
        Matcher full = EXTRACT_FULL_USER_RE.matcher(uri);
        if (full.matches()) {
            return full.group(2);
        }

        Matcher matcher = EXTRACT_USER_RE.matcher(uri);
        if (matcher.matches()) {
            return matcher.group(1);
        }

        return null;
    }

    /**
     * Extract user id and optional user info
     * 
     * <!--
     * 
     * 154@example.org => 154 sip:user@exampl.org => user "Full name"<sip:202@example.org> =>Full
     * name - 202
     * 
     * -->
     * 
     */
    public static String extractFullUser(String uri) {
        if (uri == null) {
            return null;
        }
        Matcher matcher = EXTRACT_FULL_USER_RE.matcher(uri);
        if (!matcher.matches()) {
            return null;
        }
        String fullName = matcher.group(1);
        String userId = matcher.group(2);

        if (fullName == null) {
            return userId;
        }
        return fullName + " - " + userId;
    }

    public static boolean matches(String uri) {
        Matcher matcher = SIP_URI_RE.matcher(uri);
        return matcher.matches();
    }

    public static String format(String name, String domain, Map< ? , ? > urlParams) {
        StringBuilder paramsBuffer = new StringBuilder();
        for (Map.Entry< ? , ? > entry : urlParams.entrySet()) {
            paramsBuffer.append(';');
            paramsBuffer.append(entry.getKey());
            Object value = entry.getValue();
            if (value != null) {
                paramsBuffer.append('=');
                paramsBuffer.append(value);
            }
        }

        return String.format("<sip:%s@%s%s>", name, domain, paramsBuffer);
    }

    public static String stripSipPrefix(String sipUri) {
        if (sipUri == null) {
            return null;
        }
        if (sipUri.startsWith(SIP_PREFIX)) {
            return sipUri.substring(SIP_PREFIX.length());
        }
        return sipUri;
    }
}
