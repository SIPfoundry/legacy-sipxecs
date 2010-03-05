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
    // It is sometimes important to differentiate between default port and omitted port
    public static final int OMIT_SIP_PORT = 0;

    private static final Pattern EXTRACT_USER_RE = Pattern.compile("\\s*<?(?:sip:)?(.+?)[@;].+");
    private static final Pattern EXTRACT_FULL_USER_RE = Pattern
            .compile("\\s*(?:\"?\\s*([^\"<]+?)\\s*\"?)?\\s*<(?:sip:)?(.+?)[@;].+");
    private static final Pattern SIP_URI_RE = Pattern.compile("(?:\".*\")?\\s*<?(?:sip:)?.+?[@;].+>?");

    private SipUri() {
        // utility class
    }

    public static String format(AbstractUser user, String domainName) {
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

    /**
     * Format a SIP URI from userpart, host and port
     *
     * @param userName
     * @param domainName
     * @param port - port value of 0 means omit the port from the URI
     * @return
     */
    public static String format(String userName, String domainName, int port) {
        String uri = String.format((port != OMIT_SIP_PORT) ? "sip:%s@%s:%d" : "sip:%s@%s", userName, domainName,
                port);
        return uri;
    }

    /**
     * Format a SIP URI from host and port
     *
     * @param domainName
     * @param port - port value of 0 means omit the port from the URI
     * @return
     */
    public static String format(String domainName, int port) {
        String uri = String.format((port != OMIT_SIP_PORT) ? "sip:%s:%d" : "sip:%s", domainName, port);
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

    public static String formatIgnoreDefaultPort(String displayName, String userName, String domain, int port) {
        String baseUri = formatIgnoreDefaultPort(userName, domain, port);
        if (displayName == null) {
            return baseUri;
        }
        String uri = String.format("\"%s\"<%s>", displayName, baseUri);
        return uri;
    }

    public static String format(String userName, String domain, boolean quote) {
        if (!quote) {
            return format(userName, domain, OMIT_SIP_PORT);
        }
        String format = "<sip:%s@%s>";
        return String.format(format, userName, domain);
    }

    /**
     * Format a SIP URI from userpart, host and port
     *
     * @param userName
     * @param domainName
     * @param port - port value of 0 means omit the port from the URI
     * @param qoute
     * @return
     */
    public static String format(String userName, String domainName, int port, boolean quote) {
        if (!quote) {
            return format(userName, domainName, port);
        }
        if (port == OMIT_SIP_PORT) {
            return format(userName, domainName, quote);
        }
        String format = "<sip:%s@%s:%d>";
        String uri = String.format(format, userName, domainName, port);
        return uri;
    }

    public static String normalize(String uri) {
        String result = uri.trim();
        if (result.startsWith(SIP_PREFIX)) {
            return result;
        }
        return SIP_PREFIX + result;
    }

    /**
     * Creates normailized SIP URI from the string that can represent URI or just a user part. If
     * candidate is a user part only a domain is appended to create a normailized URI.
     *
     * @param candidate SIP URI (with or without sip: prefix) or just a user part of URI
     * @param domain DNS domain appended if candidate is not a full SIP URI
     */
    public static String fix(String candidate, String domain) {
        if (StringUtils.isEmpty(candidate)) {
            return candidate;
        }
        if (matches(candidate)) {
            return normalize(candidate);
        }
        return format(candidate, domain, false);
    }

    public static String fixWithDisplayName(String candidate, String displayName, String urlParams, String domain) {
        StringBuilder uri = new StringBuilder();
        boolean needsWrapping = StringUtils.isNotBlank(displayName);
        if (needsWrapping) {
            uri.append('"');
            uri.append(displayName);
            uri.append('"');
        }

        String addParams = "";
        if (StringUtils.isNotBlank(urlParams)) {
            addParams = ";" + urlParams;
        }

        if (matches(candidate)) {
            if (needsWrapping) {
                String format = "<sip:%s>";
                uri.append(String.format(format, candidate + addParams));
            } else {
                uri.append(normalize(candidate) + addParams);
            }
        } else {
            uri.append(format(candidate, domain + addParams, needsWrapping));
        }

        return uri.toString();
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
     * Attempts to extract address-spec portion from URI
     *
     * This is rather a brute force implementation tailored to extracting URI to be used for
     * voicemail click-to-call. Rather than doing full fledged parsing it just gets the portion of
     * the string between and checks if it looks like SIP URI.
     *
     * @return address-spec or null if we cannot extract it
     */
    public static String extractAddressSpec(String uri) {
        String candidate = StringUtils.substringBetween(uri, "<", ">");
        if (matches(candidate)) {
            return candidate;
        }
        if (matches(uri)) {
            return uri;
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
            matcher = EXTRACT_USER_RE.matcher(uri);
            if (matcher.matches()) {
                return matcher.group(1);
            }
            return null;
        }
        String fullName = matcher.group(1);
        String userId = matcher.group(2);

        if (fullName == null || fullName.equals(userId)) {
            return userId;
        }
        return fullName + " - " + userId;
    }

    public static boolean matches(String uri) {
        if (uri == null) {
            return false;
        }
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

    public static String format(String name, String domain, int port, Map< ? , ? > urlParams) {
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

        return String.format("<sip:%s@%s:%d%s>", name, domain, port, paramsBuffer);
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
