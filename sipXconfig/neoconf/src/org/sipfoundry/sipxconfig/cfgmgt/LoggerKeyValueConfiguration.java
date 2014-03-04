/**
 *
 *
 * Copyright (c) 2014 Karel, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import java.io.IOException;
import java.io.Writer;

import org.sipfoundry.sipxconfig.setting.SettingUtil;

/**
 *  Custom class used to write settings for Settings which require
 *  that log.level does not get written in property file.
 *  Such is the case for java projects like sipXconfig or sipXivr
 *  which refresh their log.level directly from log4j properties.
 *  If the log.level setting is to be written in the properties file,
 *  then cf-engine will trigger a process restart, which is not the desired behaviour.
 *
 */
public class LoggerKeyValueConfiguration extends KeyValueConfiguration {

    public LoggerKeyValueConfiguration(Writer w, String delimitor, String prefix) {
        super(w, delimitor, prefix);
    }

    public static LoggerKeyValueConfiguration colonSeparated(Writer w) {
        return new LoggerKeyValueConfiguration(w, COLON, null);
    }

    public static LoggerKeyValueConfiguration colonSeparated(Writer w, String globalPrefix) {
        return new LoggerKeyValueConfiguration(w, COLON, globalPrefix);
    }

    public static LoggerKeyValueConfiguration equalsSeparated(Writer w) {
        return new LoggerKeyValueConfiguration(w, EQUALS, null);
    }

    public static LoggerKeyValueConfiguration equalsSeparated(Writer w, String globalPrefix) {
        return new LoggerKeyValueConfiguration(w, EQUALS, globalPrefix);
    }

    @Override
    public void write(String prefix, String key, Object value) throws IOException {
        if (key.equals(SettingUtil.LOG_LEVEL_SETTING_KEY)) {
            return;
        }
        super.write(prefix, key, value);
    }

}
