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

import java.util.ArrayList;
import java.util.Collection;

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
}
