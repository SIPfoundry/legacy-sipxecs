/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.dialplan.config;

import java.io.File;
import java.io.IOException;
import java.io.Writer;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.DocumentFactory;
import org.dom4j.Element;
import org.dom4j.io.SAXReader;
import org.sipfoundry.sipxconfig.dialplan.IDialingRule;

public abstract class RulesXmlFile extends RulesFile {
    protected static final DocumentFactory FACTORY = XmlFile.FACTORY;
    private static final Log LOG = LogFactory.getLog(RulesXmlFile.class);
    private String m_externalRulesFileName;


    /**
     * Trims the filename - leading and trailing spaces not supported in the file name.
     */
    public void setExternalRulesFileName(String externalRulesFileName) {
        m_externalRulesFileName = externalRulesFileName.trim();
    }

    @Override
    public void write(Writer wtr) throws IOException {
        XmlFile xml = new XmlFile(wtr);
        xml.write(getDocument());
    }

    protected abstract Document getDocument();


    /**
     * Insert mapping rules from external mapping rules file
     *
     * @param mappings - root element of the document
     */
    protected void addExternalRules(Element mappings) {
        if (m_externalRulesFileName == null) {
            return;
        }
        File externalRulesFile = new File(m_externalRulesFileName);
        if (!externalRulesFile.canRead()) {
            LOG.warn("Cannot read from external mapping rules file: " + m_externalRulesFileName);
            return;
        }
        SAXReader reader = new SAXReader();
        try {
            Document externalRules = reader.read(externalRulesFile);
            Element rootElement = externalRules.getRootElement();
            for (Iterator i = rootElement.elementIterator(); i.hasNext();) {
                Element hostMatch = (Element) i.next();
                mappings.add(hostMatch.detach());
            }
        } catch (DocumentException e) {
            LOG.error("Cannot parse external rules file", e);
        }
    }

    protected void addRuleDescription(Element userMatch, IDialingRule rule) {
        String descriptionText = rule.getDescription();
        if (!StringUtils.isBlank(descriptionText)) {
            Element description = userMatch.addElement("description");
            description.setText(descriptionText);
        }
    }

    protected void addRuleName(Element userMatch, IDialingRule rule) {
        String nameText = rule.getName();
        if (!StringUtils.isBlank(nameText)) {
            Element name = userMatch.addElement("name");
            name.setText(nameText);
        }
    }

    protected void addRuleType(Element userMatch, IDialingRule rule) {
        String typeText = rule.getRuleType();
        if (!StringUtils.isBlank(typeText)) {
            Element ruleType = userMatch.addElement("ruleType");
            ruleType.setText(typeText);
        }
    }

    protected void addRuleNameComment(Element hostMatch, IDialingRule rule) {
        String nameText = rule.getName();
        if (!StringUtils.isBlank(nameText)) {
            hostMatch.addComment(nameText);
        }
    }
}
