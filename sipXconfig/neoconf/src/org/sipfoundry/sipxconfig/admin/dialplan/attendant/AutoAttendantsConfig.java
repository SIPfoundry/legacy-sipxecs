/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.QName;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantMenuItem;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.WorkingTime.WorkingHours;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxIvrService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.springframework.beans.factory.annotation.Required;

public class AutoAttendantsConfig extends XmlFile {
    // please note: US locale always...
    private static final SimpleDateFormat HOLIDAY_FORMAT = new SimpleDateFormat("dd-MMM-yyyy", Locale.US);
    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/autoattendants-00-00";
    private static final String ID = "id";
    private static final String PARAMETER = "parameter";
    private static final Log LOG = LogFactory.getLog(AutoAttendantsConfig.class);

    private AutoAttendantManager m_autoAttendantManager;

    private DialPlanContext m_dialPlanContext;

    private DomainManager m_domainManager;

    private SipxServiceManager m_sipxServiceManager;

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        QName autoAttendantsName = FACTORY.createQName("autoattendants", NAMESPACE);
        Element aasEl = document.addElement(autoAttendantsName);
        List<AutoAttendant> autoAttendants = m_autoAttendantManager.getAutoAttendants();
        for (AutoAttendant autoAttendant : autoAttendants) {
            generateAttendants(aasEl, autoAttendant);
        }

        Element schedulesEl = aasEl.addElement("schedules");
        List<AttendantRule> attendantRules = m_dialPlanContext.getAttendantRules();
        for (AttendantRule attendantRule : attendantRules) {
            generateSchedule(schedulesEl, attendantRule);
        }
        return document;
    }

    private void generateSchedule(Element schedulesEl, AttendantRule attendantRule) {
        Element scheduleEl = schedulesEl.addElement("schedule");
        scheduleEl.addAttribute(ID, attendantRule.getSystemName());
        Holiday holidayAttendant = attendantRule.getHolidayAttendant();
        Element holidayEl = scheduleEl.addElement("holiday");
        if (holidayAttendant.isEnabled()) {
            addId(holidayEl, holidayAttendant.getAttendant());
            for (Date date : holidayAttendant.getDates()) {
                holidayEl.addElement("date").setText(HOLIDAY_FORMAT.format(date));
            }
        }
        WorkingTime workingTimeAttendant = attendantRule.getWorkingTimeAttendant();
        Element regularHoursEl = scheduleEl.addElement("regularhours");
        if (workingTimeAttendant.isEnabled()) {
            addId(regularHoursEl, workingTimeAttendant.getAttendant());
            WorkingHours[] workingHours = workingTimeAttendant.getWorkingHours();
            for (WorkingHours hours : workingHours) {
                if (hours.isEnabled()) {
                    Element dayEl = regularHoursEl.addElement(hours.getDay().getName().toLowerCase());
                    dayEl.addElement("from").setText(hours.getStartTime());
                    dayEl.addElement("to").setText(hours.getStopTime());
                }
            }
        }
        ScheduledAttendant afterHoursAttendant = attendantRule.getAfterHoursAttendant();
        Element afterHoursEl = scheduleEl.addElement("afterhours");
        if (afterHoursAttendant.isEnabled()) {
            addId(afterHoursEl, afterHoursAttendant.getAttendant());
        }
    }

    private void generateAttendants(Element aasEl, AutoAttendant autoAttendant) {
        Element aaEl = aasEl.addElement("autoattendant");
        aaEl.addAttribute(ID, autoAttendant.getSystemName());

        if (m_autoAttendantManager.getSpecialMode()
                && autoAttendant.equals(m_autoAttendantManager.getSelectedSpecialAttendant())) {
            aaEl.addAttribute("special", "true");
        }

        aaEl.addElement("name").setText(autoAttendant.getName());
        aaEl.addElement("prompt").setText(autoAttendant.getPromptFile().getPath());

        Element miEl = aaEl.addElement("menuItems");
        AttendantMenu menu = autoAttendant.getMenu();
        Map<DialPad, AttendantMenuItem> menuItems = menu.getMenuItems();
        for (Map.Entry<DialPad, AttendantMenuItem> entry : menuItems.entrySet()) {
            generateMenuItem(miEl, entry.getKey(), entry.getValue());
        }

        Element dtmfEl = aaEl.addElement("dtmf");

        // FIXME: initialTimeout parameter is actually misnamed
        // "overallDigitTimeout" which is incorrectly described,
        // as VoiceXML doesn't have such a concept.
        addSettingValueMillis(dtmfEl, "initialTimeout", autoAttendant, "dtmf/overallDigitTimeout");
        String idt = "dtmf/interDigitTimeout"; // To prevent checkStyle warning
        addSettingValueMillis(dtmfEl, "interDigitTimeout", autoAttendant, idt);
        // FIXME: extraDigitTimeout needs to be added. For now use interDigitTimeout
        addSettingValueMillis(dtmfEl, "extraDigitTimeout", autoAttendant, idt);
        addSettingValue(dtmfEl, "maximumDigits", autoAttendant, "dtmf/maxDigits");

        Element irEl = aaEl.addElement("invalidResponse");
        addSettingValue(irEl, "noInputCount", autoAttendant, "onfail/noinputCount");
        addSettingValue(irEl, "invalidResponseCount", autoAttendant, "onfail/nomatchCount");
        Boolean transfer = (Boolean) autoAttendant.getSettingTypedValue("onfail/transfer");
        irEl.addElement("transferOnFailures").setText(transfer.toString());
        if (transfer.booleanValue()) {
            String value = autoAttendant.getSettingValue("onfail/transfer-extension");
            if (value != null) {
                String transferUrl = SipUri.fix(value, getDomainName());
                irEl.addElement("transferUrl").setText(transferUrl);
            }
            String transferPromptValue = autoAttendant.getSettingValue("onfail/transfer-prompt");
            if (transferPromptValue != null) {
                String promptsDirectory = autoAttendant.getPromptsDirectory();
                File fullPathTransferPromptFile = new File(promptsDirectory, transferPromptValue);
                irEl.addElement("transferPrompt").setText(fullPathTransferPromptFile.getPath());
            }
        }
    }

    /**
     * Retrieves the setting value, rescales it to millis (multiplying by 1000), and adds to
     * element.
     *
     * @param parent element to which new element is added
     * @param name the name of newly added element
     * @param bean the bean from which setting value is read
     * @param settingName the name of the setting; in this case it has to be integer setting
     *        expressed in seconds
     */
    private void addSettingValueMillis(Element parent, String name, BeanWithSettings bean, String settingName) {
        Integer value = (Integer) bean.getSettingTypedValue(settingName);
        if (value != null) {
            long millisValue = value * 1000;
            parent.addElement(name).setText(Long.toString(millisValue));
        }
    }

    private void addSettingValue(Element parent, String name, BeanWithSettings bean, String settingName) {
        String value = bean.getSettingValue(settingName);
        if (value != null) {
            parent.addElement(name).setText(value);
        }
    }

    private void addId(Element aaEl, AutoAttendant autoAttendant) {
        aaEl.addElement(ID).setText(autoAttendant.getSystemName());
    }

    private void generateMenuItem(Element misEl, DialPad dialPad, AttendantMenuItem menuItem) {
        Element miEl = FACTORY.createElement("menuItem", NAMESPACE);

        miEl.addElement("dialPad").setText(dialPad.getName());
        AttendantMenuAction action = menuItem.getAction();
        miEl.addElement("action").setText(action.getName());

        if (action.isVoicemailParameter()) {
            String voicemailUrl = getVoicemailUrl();
            if (voicemailUrl == null) {
                logNullParameterError();
                return;
            }
            miEl.addElement(PARAMETER).setText(voicemailUrl);
        }

        String parameter = menuItem.getParameter();
        if (action.isAttendantParameter()) {
            if (parameter == null) {
                logNullParameterError();
                return;
            }
            miEl.addElement(PARAMETER).setText(parameter);
        }
        if (action.isExtensionParameter()) {
            if (parameter == null) {
                logNullParameterError();
                return;
            }
            miEl.addElement("extension").setText(parameter);
        }

        misEl.add(miEl);
    }

    public String getVoicemailUrl() {
        String voiceMail = m_dialPlanContext.getVoiceMail();
        return SipUri.fix(voiceMail, getDomainName());
    }

    public String getDomainName() {
        return m_domainManager.getDomain().getName();
    }

    @Override
    public boolean isReplicable(Location location) {
        return m_sipxServiceManager.isServiceInstalled(location.getId(), SipxIvrService.BEAN_ID);
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Required
    public void setAutoAttendantManager(AutoAttendantManager autoAttendantManager) {
        m_autoAttendantManager = autoAttendantManager;
    }

    @Required
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    private void logNullParameterError() {
        LOG.warn("Menu item's parameter is null. "
                + "The generation of autoattendants.xml file will ignore this parameter and continue.");
    }
}
