/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;

public final class SettingUtil {
    private SettingUtil() {
    }

    /**
     * Non inclusive (e.g. will never include root setting in collection) User a filter to
     * generate a collection. Runner will naturally recurse tree up to filter to accept/reject
     * settings. For chaining filters, try creating a composite filter instead for running
     * collections back into this runner.
     *
     * <pre>
     *             Example:
     *               class MyClass {
     *
     *                    private static final SettingFilter MY_SETTING = new SettingFilter() {
     *                        public boolean acceptSetting(Setting root_, Setting setting) {
     *                            return setting.getName().equals(&quot;mysetting&quot;);
     *                        }
     *                    };
     *
     *                   public Collection getMySettings(Setting settings) {
     *                         return SettingUtil.filter(MY_SETTINGS, settings);
     *                   }
     *               }
     * </pre>
     */
    public static Collection<Setting> filter(SettingFilter filter, Setting root) {
        FilterRunner runner = new FilterRunner(filter, root);
        for (Setting s : root.getValues()) {
            s.acceptVisitor(runner);
        }
        return runner.m_collection;
    }

    /**
     * If a setting set is advanced, then all it's children can be considered advanced. USE CASE :
     * XCF-751
     *
     * @param node parent to some level of setting
     * @param setting descendant of parent
     * @return true if any node is advanced including node itself
     */
    public static boolean isAdvancedIncludingParents(Setting node, Setting setting) {
        for (Setting s = setting; s != null; s = s.getParent()) {
            if (s.isAdvanced()) {
                return true;
            }
            if (s == node) {
                break;
            }
        }
        return false;
    }

    static class FilterRunner implements SettingVisitor {
        private Collection<Setting> m_collection = new ArrayList<Setting>();

        private SettingFilter m_filter;

        private Setting m_root;

        FilterRunner(SettingFilter filter, Setting root) {
            m_filter = filter;
            m_root = root;
        }

        public void visitSetting(Setting setting) {
            if (m_filter.acceptSetting(m_root, setting)) {
                m_collection.add(setting);
            }
        }

        public boolean visitSettingGroup(SettingSet settingGroup) {
            visitSetting(settingGroup);
            return true;
        }

        public boolean visitSettingArray(SettingArray array) {
            visitSetting(array);
            return true;
        }
    }

    /**
     * When you have a setting or group of settings that form an ip address or
     * set of ip addresses.  This abstracts the definition so of underlying settings
     * definition changes to or from a group, this still works.
     *
     * This does not include port numbers, ports on addresses are zero (i.e. unset and
     * therefore the implicit default for that address)
     *
     * Example:
     *   SettingUtil.getAddresses(getSettings(), "named/forwarders");
     *
     */
    public static List<Address> getAddresses(AddressType t, Setting base, String setting) {
        List<Address> addresses = Collections.emptyList();
        Setting s = base.getSetting(setting);
        if (s instanceof SettingSet) {
            SettingSet set = (SettingSet) base.getSetting(setting);
            Collection<Setting> values = set.getValues();
            addresses = new ArrayList<Address>();
            for (Setting server : values) {
                String value = server.getValue();
                if (value != null) {
                    addresses.add(new Address(t, value));
                }
            }
        } else {
            String value = s.getValue();
            if (value != null) {
                addresses = Collections.singletonList(new Address(t, value));
            }
        }

        return addresses;
    }

    /**
     * Writes log4j log level in a specified file.
     */
    public static void writeLog4jSetting(Setting settings, File dir,
            String fileName) throws IOException {
        writeLog4jSetting(settings, dir, fileName, true, SipFoundryLayout.LOG4J_SIPFOUNDRY_KEY);
    }

    /**
     * Writes log4j log level in a specified file with a given key.
     */
    public static void writeLog4jSetting(Setting settings, File dir,
            String fileName, String logLevelKey) throws IOException {
        writeLog4jSetting(settings, dir, fileName, true, logLevelKey);
    }

    /**
     * Writes log4j log level in a specified file and, if needed,
     * removes the log.level setting from the settings object.
     */
    public static void writeLog4jSetting(Setting settings, File dir,
            String fileName, boolean removeLogLevelSetting, String logLevelKey) throws IOException {
        Setting logLevelSettings = settings.getSetting("log.level");
        File f = new File(dir, fileName);
        Writer wtr = new FileWriter(f);
        try {
            KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(wtr);
            String log4jLogLevel = SipFoundryLayout.mapSipFoundry2log4j(logLevelSettings.getValue()).toString();
            logLevelSettings.setValue(log4jLogLevel);
            config.writeWithKey(logLevelKey, logLevelSettings);
            if (removeLogLevelSetting) {
                settings.getValues().remove(logLevelSettings);
            }
        } finally {
            IOUtils.closeQuietly(wtr);
        }
    }
}
