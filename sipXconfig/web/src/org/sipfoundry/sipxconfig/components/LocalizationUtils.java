/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.components;

import java.util.Locale;

import org.apache.hivemind.Messages;
import org.apache.tapestry.IComponent;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.context.MessageSource;

public final class LocalizationUtils {
    // Makes checkstyle happy
    private static final String DEPRECATION = "deprecation";

    private LocalizationUtils() {
        // utility only class
    }

    /**
     * Tapestry4 does not provide a way of detecting if message has been provided. This method is
     * trying to guess if message has been retrieved or if Tapestry is trying to help by providing
     * a formatted key as a message.
     *
     * @param defaultValue value that should be used if key is not found
     * @return found message or default value if key not found
     */
    public static final String getMessage(Messages messages, String key, String defaultValue) {
        String candidate = messages.getMessage(key);
        if (candidate.equalsIgnoreCase("[" + key + "]")) {
            return defaultValue;
        }
        return candidate;
    }

    /**
     * Convience method that will quietly localize and act graceful if resources are not set or
     * found.
     */
    public static final String localize(Messages messages, String key) {
        return LocalizationUtils.localize(messages, null, key);
    }

    /**
     * @param prefix prepended to key w/o '.' (e.g. fullkey = prefix + key)
     */
    public static final String localize(Messages messages, String prefix, String key) {
        if (key == null || messages == null) {
            return key;
        }

        String fullKey = prefix != null ? prefix + key : key;
        String label = messages.getMessage(fullKey);
        return label;
    }

    /**
     * Retrieves localized version of the key.
     *
     * It uses Spring MessageSource and not Hivemind Messages class that are typical in Tapestry.
     * We are loading resource bundles in neoconf project when parsing the models and I did not
     * want introduce dependency on Tapestry there. (We of course already have Spring dependency
     * in neoconf.) Tapestry localization is slightly more flexible because it can handle
     * properties file in various encoding including UTF-8. However we stick to Java default
     * encoding for the moment, using default Java classes should be OK.
     *
     * @param current component - used to retrieve page locale
     * @param message source - if null defaultMessage is returned
     * @param key message key
     * @param defaultMessage return id localized version is not found
     * @return localized message for the key
     */
    public static String getModelMessage(IComponent component, MessageSource modelMessages, String key,
            String defaultMessage) {
        if (modelMessages != null) {
            Locale locale = component.getPage().getLocale();
            return modelMessages.getMessage(key, null, defaultMessage, locale);
        }
        return defaultMessage;
    }

    @SuppressWarnings(DEPRECATION)
    public static String getSettingLabel(IComponent component, Setting setting) {
        return getModelMessage(component, setting.getMessageSource(), setting.getLabelKey(), setting.getLabel());
    }

    @SuppressWarnings(DEPRECATION)
    public static String getSettingDescription(IComponent component, Setting setting) {
        return getModelMessage(component, setting.getMessageSource(), setting.getDescriptionKey(), setting
                .getDescription());
    }

    /**
     * Localizes String that start with '&'. Does not localize other strings. Does not localize
     * things that are not strings.
     */
    public static <T> T localizeString(Messages messages, T item) {
        if (!(item instanceof String)) {
            // can only localize Strings
            return item;
        }
        String str = (String) item;
        if (!str.startsWith("&")) {
            // not meant to be localized
            return item;
        }
        String key = str.substring(1);
        return (T) messages.getMessage(key);
    }

    /**
     * Attempts to localize UserException by localizing message and all string parameters if they
     * have '&' prefix.
     */
    public static String localizeException(Messages messages, Throwable t) {
        if (!(t instanceof UserException)) {
            return t.getLocalizedMessage();
        }
        UserException e = (UserException) t;
        String msg = e.getRawMessage();
        if (msg == null) {
            return e.getMessage();
        }
        String localizedMsg = localizeString(messages, msg);
        Object[] localizedParams = LocalizationUtils.localizeArray(messages, e.getRawParams());
        return e.format(localizedMsg, localizedParams);
    }

    static Object[] localizeArray(Messages messages, Object... params) {
        if (params == null) {
            return null;
        }
        Object[] localizedParams = new Object[params.length];
        for (int i = 0; i < params.length; i++) {
            localizedParams[i] = localizeString(messages, params[i]);
        }
        return localizedParams;
    }
}
