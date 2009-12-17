/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.digester.BeanPropertySetterRule;
import org.apache.commons.digester.Digester;
import org.apache.commons.digester.Rule;
import org.apache.commons.digester.RuleSetBase;
import org.apache.commons.digester.SetNestedPropertiesRule;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;
import org.sipfoundry.sipxconfig.setting.type.EnumSetting;
import org.sipfoundry.sipxconfig.setting.type.FileSetting;
import org.sipfoundry.sipxconfig.setting.type.HostnameSetting;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;
import org.sipfoundry.sipxconfig.setting.type.IpAddrSetting;
import org.sipfoundry.sipxconfig.setting.type.MultiEnumSetting;
import org.sipfoundry.sipxconfig.setting.type.RealSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;
import org.sipfoundry.sipxconfig.setting.type.SipUriSetting;
import org.sipfoundry.sipxconfig.setting.type.StringSetting;
import org.xml.sax.Attributes;
import org.xml.sax.EntityResolver;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 * Build a SettingModel object hierarchy from a model XML file.
 */
public class XmlModelBuilder implements ModelBuilder {
    private static final String ADD_SETTING_METHOD = "addSetting";
    private static final String EL_VALUE = "/value";
    private static final String EL_LABEL = "/label";
    private static final String REQUIRED = "required";
    private static final String EL_OPTION = "/option";
    private static final String ADD_ENUM_METHOD = "addEnum";

    private final File m_configDirectory;

    public XmlModelBuilder(File configDirectory) {
        m_configDirectory = configDirectory;
    }

    public XmlModelBuilder(String configDirectory) {
        this(new File(configDirectory));
    }

    public SettingSet buildModel(File modelFile) {
        FileInputStream is = null;
        try {
            is = new FileInputStream(modelFile);
            SettingSet model = buildModel(is, modelFile.getParentFile());
            ModelMessageSource messageSource = new ModelMessageSource(modelFile);
            model.setMessageSource(messageSource);
            return model;

        } catch (IOException e) {
            throw new RuntimeException("Cannot parse model definitions file " + modelFile.getPath(), e);
        } finally {
            IOUtils.closeQuietly(is);
        }
    }

    private SettingSet buildModel(InputStream is, File baseSystemId) throws IOException {
        Digester digester = new Digester();

        // setting classloader ensures classes are searched for in this classloader
        // instead of parent's classloader is digister was loaded there.
        digester.setClassLoader(getClass().getClassLoader());
        digester.setValidating(false);
        EntityResolver entityResolver = new ModelEntityResolver(m_configDirectory, baseSystemId);
        digester.setEntityResolver(entityResolver);
        digester.push(new ConditionalSet());

        // keeps all types encountered during parsing
        SettingTypeIdRule typeIdRule = new SettingTypeIdRule();

        addSettingTypes(digester, "model/type/", typeIdRule);

        CollectionRuleSet collectionRule = new CollectionRuleSet();
        digester.addRuleSet(collectionRule);

        SettingRuleSet groupRule = new SettingRuleSet("*/group", ConditionalSet.class, typeIdRule);
        digester.addRuleSet(groupRule);

        SettingRuleSet settingRule = new SettingRuleSet("*/setting", ConditionalSettingImpl.class, typeIdRule);
        digester.addRuleSet(settingRule);

        try {
            return (SettingSet) digester.parse(is);
        } catch (SAXException se) {
            throw new RuntimeException("Could not parse model definition file", se);
        }
    }

    private static void addSettingTypes(Digester digester, String patternPrefix, SettingTypeIdRule typeIdRule) {
        digester.addRuleSet(new IntegerSettingRule(patternPrefix + "integer", typeIdRule));
        digester.addRuleSet(new RealSettingRule(patternPrefix + "real", typeIdRule));
        digester.addRuleSet(new StringSettingRule(patternPrefix + "string", typeIdRule));
        digester.addRuleSet(new EnumSettingRule(patternPrefix + "enum", typeIdRule));
        digester.addRuleSet(new MultiEnumSettingRule(patternPrefix + "multi-enum", typeIdRule));
        digester.addRuleSet(new BooleanSettingRule(patternPrefix + "boolean", typeIdRule));
        digester.addRuleSet(new FileSettingRule(patternPrefix + "file", typeIdRule));
        digester.addRuleSet(new SpecializedStringSettingRule(patternPrefix + "sip-uri", typeIdRule,
                SipUriSetting.class));
        digester.addRuleSet(new SpecializedStringSettingRule(patternPrefix + "ipaddr", typeIdRule,
                IpAddrSetting.class));
        digester.addRuleSet(new SpecializedStringSettingRule(patternPrefix + "hostname", typeIdRule,
                HostnameSetting.class));
    }

    static class AbstractSettingRuleSet extends RuleSetBase {
        private final String m_pattern;
        private final Class m_klass;

        public AbstractSettingRuleSet(String pattern, Class klass) {
            m_pattern = pattern;
            m_klass = klass;
        }

        @Override
        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(m_pattern, m_klass);
            digester.addSetProperties(m_pattern, "parent", null);
            final String[] properties = {
                "/description", "/profileName", EL_LABEL
            };
            for (int i = 0; i < properties.length; i++) {
                digester.addBeanPropertySetter(m_pattern + properties[i]);
            }
        }

        protected String getPattern() {
            return m_pattern;
        }
    }

    static class CollectionRuleSet extends AbstractSettingRuleSet {
        public CollectionRuleSet() {
            super("*/collection", SettingArray.class);
        }

        @Override
        public void addRuleInstances(Digester digester) {
            super.addRuleInstances(digester);
            digester.addSetNext(getPattern(), ADD_SETTING_METHOD, Setting.class.getName());
        }
    }

    static class SettingRuleSet extends AbstractSettingRuleSet {
        private final SettingTypeIdRule m_typeIdRule;

        public SettingRuleSet(String pattern, Class klass, SettingTypeIdRule typeIdRule) {
            super(pattern, klass);
            m_typeIdRule = typeIdRule;
        }

        @Override
        public void addRuleInstances(Digester digester) {
            super.addRuleInstances(digester);
            digester.addRule(getPattern(), new CopyOfRule());
            digester.addRule(getPattern() + EL_VALUE, new BeanPropertyNullOnEmptyStringRule("value"));
            addSettingTypes(digester, getPattern() + "/type/", m_typeIdRule);
            digester.addSetNext(getPattern(), ADD_SETTING_METHOD, ConditionalSettingImpl.class.getName());
        }
    }

    static class BeanPropertyNullOnEmptyStringRule extends BeanPropertySetterRule {
        public BeanPropertyNullOnEmptyStringRule(String property) {
            super(property);
        }

        @Override
        public void body(String namespace, String name, String text) throws Exception {

            super.body(namespace, name, text);
            if (StringUtils.isEmpty(bodyText)) {
                bodyText = null;
            }
        }
    }

    static class CopyOfRule extends Rule {

        @Override
        public void begin(String namespace_, String name_, Attributes attributes) {
            // warning, this is called TWICE by digester! and I do not know why. Stack
            // is identical so i think it's coming from SAXParser. Code here works fine,
            // but heed warning
            String copyOfName = attributes.getValue("copy-of");
            if (copyOfName != null) {
                ConditionalSetting copyTo = (ConditionalSetting) getDigester().pop();
                Setting parent = (Setting) getDigester().peek();
                // setting to be copied must be defined in file before setting
                // attempting to copy
                Setting copyOf = parent.getSetting(copyOfName);
                Setting copy = copyOf.copy();
                copy.setName(copyTo.getName());

                // copies will explicitly not inherit if/unless
                // i think it's against intuition -- DLH 12/03/05
                ConditionalSetting s = (ConditionalSetting) copy;
                s.setIf(copyTo.getIf());
                s.setUnless(copyTo.getUnless());

                getDigester().push(copy);
            }
        }
    }

    static class SettingTypeIdRule extends Rule {
        private String m_id;

        private final Map m_types = new HashMap();

        @Override
        public void end(String namespace_, String name_) {
            if (m_id != null) {
                Setting rootSetting = (Setting) getDigester().peek();
                SettingType type = rootSetting.getType();
                type.setId(m_id);
                m_types.put(m_id, type);
            }
        }

        @Override
        public void begin(String namespace_, String name_, Attributes attributes) {
            m_id = attributes.getValue("id");
            String refid = attributes.getValue("refid");
            if (refid != null) {
                SettingType prototype = (SettingType) m_types.get(refid);
                if (prototype == null) {
                    throw new IllegalArgumentException("Setting type with id=" + refid + " not found.");
                }
                SettingType type = prototype.clone();
                Setting setting = (Setting) getDigester().peek();
                setting.setType(type);

                String required = attributes.getValue(REQUIRED);
                if (!StringUtils.isBlank(required)) {
                    try {
                        BeanUtils.setProperty(type, REQUIRED, "yes".equals(required));
                    } catch (IllegalAccessException e) {
                        throw new IllegalArgumentException("Could not access 'required' property on " + type);
                    } catch (InvocationTargetException e) {
                        throw new IllegalArgumentException("Could not set 'required' property on " + type);
                    }
                }
            }
        }
    }

    static class SettingTypeRule extends RuleSetBase {
        private final String m_pattern;

        private final SettingTypeIdRule m_typeIdRule;

        public SettingTypeRule(String pattern, SettingTypeIdRule typeIdRule) {
            m_pattern = pattern;
            m_typeIdRule = typeIdRule;
        }

        @Override
        public void addRuleInstances(Digester digester) {
            digester.addSetNext(m_pattern, "setType", SettingType.class.getName());
            digester.addRule(getParentPattern(), m_typeIdRule);
        }

        String getParentPattern() {
            int slash = m_pattern.lastIndexOf('/');
            return m_pattern.substring(0, slash);
        }

        public String getPattern() {
            return m_pattern;
        }
    }

    static class StringSettingRule extends SettingTypeRule {
        public StringSettingRule(String pattern, SettingTypeIdRule typeIdRule) {
            super(pattern, typeIdRule);
        }

        @Override
        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(getPattern(), StringSetting.class);
            digester.addSetProperties(getPattern());
            digester.addBeanPropertySetter(getPattern() + "/pattern");
            super.addRuleInstances(digester);
        }
    }

    static class IntegerSettingRule extends SettingTypeRule {
        public IntegerSettingRule(String pattern, SettingTypeIdRule typeIdRule) {
            super(pattern, typeIdRule);
        }

        @Override
        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(getPattern(), IntegerSetting.class);
            digester.addSetProperties(getPattern());
            super.addRuleInstances(digester);
        }
    }

    static class RealSettingRule extends SettingTypeRule {
        public RealSettingRule(String pattern, SettingTypeIdRule typeIdRule) {
            super(pattern, typeIdRule);
        }

        @Override
        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(getPattern(), RealSetting.class);
            digester.addSetProperties(getPattern());
            super.addRuleInstances(digester);
        }
    }

    static class BooleanSettingRule extends SettingTypeRule {
        public BooleanSettingRule(String pattern, SettingTypeIdRule typeIdRule) {
            super(pattern, typeIdRule);
        }

        @Override
        public void addRuleInstances(Digester digester) {
            String pattern = getPattern();
            digester.addObjectCreate(pattern, BooleanSetting.class);
            digester.addSetProperties(pattern);
            digester.addBeanPropertySetter(pattern + "/true/value", "trueValue");
            digester.addBeanPropertySetter(pattern + "/false/value", "falseValue");
            super.addRuleInstances(digester);
        }
    }

    abstract static class OptionsSettingRule extends SettingTypeRule {
        private final Class m_klass;

        public OptionsSettingRule(String pattern, SettingTypeIdRule typeIdRule, Class klass) {
            super(pattern, typeIdRule);
            m_klass = klass;
        }

        @Override
        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(getPattern(), m_klass);
            digester.addSetProperties(getPattern());
            String option = getPattern() + EL_OPTION;
            digester.addCallMethod(option, ADD_ENUM_METHOD, 2);
            digester.addCallParam(option + EL_VALUE, 0);
            digester.addCallParam(option + EL_LABEL, 1);
            super.addRuleInstances(digester);
        }
    }

    static class EnumSettingRule extends OptionsSettingRule {
        public EnumSettingRule(String pattern, SettingTypeIdRule typeIdRule) {
            super(pattern, typeIdRule, EnumSetting.class);
        }
    }

    static class MultiEnumSettingRule extends OptionsSettingRule {
        public MultiEnumSettingRule(String pattern, SettingTypeIdRule typeIdRule) {
            super(pattern, typeIdRule, MultiEnumSetting.class);
        }
    }

    static class FileSettingRule extends SettingTypeRule {
        public FileSettingRule(String pattern, SettingTypeIdRule typeIdRule) {
            super(pattern, typeIdRule);
        }

        @Override
        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(getPattern(), FileSetting.class);
            digester.addSetProperties(getPattern());

            SetNestedPropertiesRule nestedPropertiesRule = new SetNestedPropertiesRule();
            nestedPropertiesRule.setAllowUnknownChildElements(true);
            digester.addRule(getPattern(), nestedPropertiesRule);

            String zipexclude = getPattern() + "/exclude/filename";
            digester.addCallMethod(zipexclude, "addZipExclude", 1);
            digester.addCallParam(zipexclude, 0);
            super.addRuleInstances(digester);
        }
    }

    static class SpecializedStringSettingRule extends SettingTypeRule {
        private final Class m_klass;

        public SpecializedStringSettingRule(String pattern, SettingTypeIdRule typeIdRule, Class klass) {
            super(pattern, typeIdRule);
            m_klass = klass;
        }

        @Override
        public void addRuleInstances(Digester digester) {
            digester.addObjectCreate(getPattern(), m_klass);
            digester.addSetProperties(getPattern());
            digester.addSetNestedProperties(getPattern());
            super.addRuleInstances(digester);
        }
    }

    private static class ModelEntityResolver implements EntityResolver {
        private static final Log LOG = LogFactory.getLog(ModelEntityResolver.class);
        private static final String DTD = "setting.dtd";

        private final File m_dtd;
        private final File m_baseSystemId;

        ModelEntityResolver(File configDirectory, File baseSystemId) {
            m_dtd = new File(configDirectory, DTD);
            m_baseSystemId = baseSystemId;
        }

        public InputSource resolveEntity(String publicId, String systemId) throws IOException {
            if (publicId != null) {
                if (publicId.startsWith("-//SIPFoundry//sipXconfig//Model specification ")) {
                    InputStream dtdStream = null;
                    if (m_dtd.exists()) {
                        dtdStream = new FileInputStream(m_dtd);
                    } else {
                        LOG.warn("Cannot find " + m_dtd);
                        // try classpath
                        dtdStream = getClass().getClassLoader().getResourceAsStream(DTD);

                    }
                    if (dtdStream != null) {
                        return new InputSource(dtdStream);
                    }
                    // FIXME: this usually requires internet connection
                    return new InputSource(systemId);
                }
            } else if (systemId != null && m_baseSystemId != null) {
                // LIMITATION: All files loaded as ENTITYies defined as SYSTEM
                // must live in same directory as XML file
                //
                // HACK: Xerces 2.7.0 has a propensity to expand systemId to full path
                // which makes it impossible to determine original relative URI. Tricks to use
                // systemId on inputsource and using file:// failed.
                String name = new File(systemId).getName();
                File f = new File(m_baseSystemId, name);
                return new InputSource(new FileInputStream(f));
            }

            return null;
        }
    }
}
