/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Collection;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

public class Schema {
    public static class ClassDefinition {
        public static final Log LOG = LogFactory.getLog(ClassDefinition.class);

        private static final String PARAM_SEP = " $";
        private static final Pattern NAME_PATTERN = Pattern.compile("NAME '([;\\-\\w]+)'(?:\\s*DESC\\s*'(.*)')?");
        private static final Pattern SUP_PATTERN = Pattern.compile("SUP ([;\\-\\w]+) ");
        private static final Pattern MUST_PATTERN = Pattern.compile("MUST \\(\\s*([;\\-\\w\\s$]+)\\s*\\)");
        private static final Pattern MUST_SINGLE_PATTERN = Pattern.compile("MUST ([;\\-\\w]+)\\s*");
        private static final Pattern MAY_PATTERN = Pattern.compile("MAY \\(\\s*([;\\-\\w\\s$]+)\\s*\\)");
        private static final Pattern MAY_SINGLE_PATTERN = Pattern.compile("MAY ([;\\-\\w]+)\\s*");

        private String m_name;
        private String m_description;
        private String m_sup;
        private String[] m_must = ArrayUtils.EMPTY_STRING_ARRAY;
        private String[] m_may = ArrayUtils.EMPTY_STRING_ARRAY;

        public ClassDefinition(String name, String description) {
            m_name = name;
            m_description = description;
        }

        public String[] getAllAttributes() {
            return (String[]) ArrayUtils.addAll(m_must, m_may);
        }

        public String getDescription() {
            return m_description;
        }

        public void setDescription(String description) {
            m_description = description;
        }

        public String[] getMay() {
            return m_may;
        }

        public void setMay(String[] may) {
            m_may = may;
        }

        public String[] getMust() {
            return m_must;
        }

        public void setMust(String[] must) {
            m_must = must;
        }

        public String getName() {
            return m_name;
        }

        public void setName(String name) {
            m_name = name;
        }

        public String getSup() {
            return m_sup;
        }

        public void setSup(String sup) {
            m_sup = sup;
        }

        public static ClassDefinition fromSchemaString(String classDefintionString) {
            Matcher nameMatcher = NAME_PATTERN.matcher(classDefintionString);
            if (!nameMatcher.find()) {
                LOG.info("Cannot parse:" + classDefintionString);
                return null;
            }

            String objectClass = nameMatcher.group(1);
            String description = nameMatcher.group(2);
            ClassDefinition definition = new ClassDefinition(objectClass, description);

            int newStart = nameMatcher.end();

            Matcher supMatcher = SUP_PATTERN.matcher(classDefintionString);
            if (supMatcher.find(newStart)) {
                String sup = supMatcher.group(1);
                definition.setSup(sup);

                newStart = supMatcher.end();
            }

            Matcher mustMatcher = MUST_PATTERN.matcher(classDefintionString);
            Matcher mustSingleMatcher = MUST_SINGLE_PATTERN.matcher(classDefintionString);
            if (mustMatcher.find(newStart)) {
                String mustParamsString = mustMatcher.group(1);
                String[] mustParams = StringUtils.split(mustParamsString, PARAM_SEP);
                definition.setMust(mustParams);

                newStart = mustMatcher.end();
            } else if (mustSingleMatcher.find(newStart)) {
                String mustParam = mustSingleMatcher.group(1);
                definition.setMust(new String[] {
                    mustParam
                });

                newStart = mustSingleMatcher.end();
            }

            Matcher mayMatcher = MAY_PATTERN.matcher(classDefintionString);
            Matcher maySingleMatcher = MAY_SINGLE_PATTERN.matcher(classDefintionString);
            if (mayMatcher.find(newStart)) {
                String mayParamsString = mayMatcher.group(1);
                String[] mayParams = StringUtils.split(mayParamsString, PARAM_SEP);
                definition.setMay(mayParams);
            } else if (maySingleMatcher.find(newStart)) {
                String mayParam = maySingleMatcher.group(1);
                definition.setMay(new String[] {
                    mayParam
                });
            }

            return definition;
        }
    }

    private Map<String, ClassDefinition> m_schema = new TreeMap<String, ClassDefinition>();

    public void addClassDefinition(String classDefinition) {
        ClassDefinition definition = ClassDefinition.fromSchemaString(classDefinition);
        if (definition != null) {
            m_schema.put(definition.getName(), definition);
        }
    }

    public String[] getAttributes(String objectClass) {
        ClassDefinition definition = m_schema.get(objectClass);
        if (definition == null) {
            return null;
        }
        return definition.getAllAttributes();
    }

    public String[] getObjectClassesNames() {
        return m_schema.keySet().toArray(new String[m_schema.size()]);
    }

    public String[] getAttributesPool(Collection<String> objectClasses) {
        Set<String> attributesPool = new TreeSet<String>();
        for (String objectClass : objectClasses) {
            String[] attributes = getAttributes(objectClass);
            if (attributes == null) {
                continue;
            }
            for (String attribute : attributes) {
                attributesPool.add(attribute);
            }
        }
        return attributesPool.toArray(new String[attributesPool.size()]);
    }
}
