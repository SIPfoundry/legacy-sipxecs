/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.attendant;

import java.lang.reflect.InvocationTargetException;
import java.util.List;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.dom4j.Document;
import org.dom4j.Element;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.phonebook.Address;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.service.SipxImbotService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.common.DaoUtils.forAllUsersDo;

public class ContactInformationConfig extends XmlFile {
    private static final Log LOG = LogFactory.getLog(ContactInformationConfig.class);

    private static final String NAMESPACE = "http://www.sipfoundry.org/sipX/schema/xml/contactinfo-00-00";
    private CoreContext m_coreContext;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private SipxServiceManager m_sipxServiceManager;

    @Override
    public Document getDocument() {
        Document document = FACTORY.createDocument();
        final Element contactInfos = document.addElement("contact-info", NAMESPACE);

        Closure<User> closure = new Closure<User>() {
            @Override
            public void execute(User user) {
                generateUser(user, contactInfos);
            }
        };
        forAllUsersDo(m_coreContext, closure);
        return document;
    }

    private void generateUser(User user, Element element) {
        Element userEl = element.addElement("user");
        userEl.addElement("userName").setText(user.getUserName());

        ImAccount imAccount = new ImAccount(user);
        addElements(userEl, imAccount, "imId", "imDisplayName");

        AddressBookEntry abe = user.getAddressBookEntry();
        if (abe != null) {
            //FIXME abe.getOfficeAddress should be accurate enough to get real office address
            //complete fix should be when XX-8002 gets solved
            Address officeAddress = null;
            Branch site = user.getSite();
            if (abe.getUseBranchAddress() && site != null) {
                officeAddress = site.getAddress();
            } else {
                officeAddress = abe.getOfficeAddress();
            }

            addElements(userEl, abe, "alternateImId", "jobTitle", "jobDept", "companyName", "assistantName",
                    "assistantPhoneNumber", "faxNumber", "location", "homePhoneNumber", "cellPhoneNumber");

            Element homeAddressEl = userEl.addElement("homeAddress");
            addAddressInfo(homeAddressEl, abe.getHomeAddress());

            Element officeAddressEl = userEl.addElement("officeAddress");
            addAddressInfo(officeAddressEl, officeAddress);
        }

        List<Conference> conferences = m_conferenceBridgeContext.findConferencesByOwner(user);
        Element conferencesEl = userEl.addElement("conferences");
        for (Conference conference : conferences) {
            Element conferenceElement = conferencesEl.addElement("conference");
            // conference name and extension are required for a conference thus we need not to
            // check for null/empty here
            conferenceElement.addElement("name").setText(conference.getName());
            conferenceElement.addElement("extension").setText(conference.getExtension());
            if (StringUtils.isNotEmpty(conference.getParticipantAccessCode())) {
                conferenceElement.addElement("pin").setText(conference.getParticipantAccessCode());
            }
        }

        // Adding settings used by PersonalAssistant service.
        String conferenceEntryIM = user.getSettingValue("im_notification/conferenceEntryIM").toString();
        String conferenceExitIM = user.getSettingValue("im_notification/conferenceExitIM").toString();
        String leaveMsgBeginIM = user.getSettingValue("im_notification/leaveMsgBeginIM").toString();
        String leaveMsgEndIM = user.getSettingValue("im_notification/leaveMsgEndIM").toString();
        userEl.addElement("conferenceEntryIM").setText(conferenceEntryIM);
        userEl.addElement("conferenceExitIM").setText(conferenceExitIM);
        userEl.addElement("leaveMsgBeginIM").setText(leaveMsgBeginIM);
        userEl.addElement("leaveMsgEndIM").setText(leaveMsgEndIM);
    }

    private void addAddressInfo(Element element, Address address) {
        if (address != null) {
            addElements(element, address, "street", "city", "country", "state", "zip", "officeDesignation");
        }
    }

    private void addElement(Element userEl, Object bean, String name) {
        try {
            String value = BeanUtils.getSimpleProperty(bean, name);
            if (!StringUtils.isEmpty(value)) {
                userEl.addElement(name).setText(value);
            }
        } catch (IllegalAccessException e) {
            LOG.error(e);
        } catch (InvocationTargetException e) {
            LOG.error(e);
        } catch (NoSuchMethodException e) {
            LOG.error(e);
        }
    }

    private void addElements(Element userEl, Object bean, String... names) {
        for (String name : names) {
            addElement(userEl, bean, name);
        }
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setSipxServiceManager(SipxServiceManager sipxServiceManager) {
        m_sipxServiceManager = sipxServiceManager;
    }

    @Required
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    @Override
    public boolean isReplicable(Location location) {
        return m_sipxServiceManager.isServiceInstalled(location.getId(), SipxImbotService.BEAN_ID)
                && m_sipxServiceManager.getServiceByBeanId(SipxImbotService.BEAN_ID).isAvailable();
    }
}
