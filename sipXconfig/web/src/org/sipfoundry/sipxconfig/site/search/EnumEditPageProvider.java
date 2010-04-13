/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.search;

import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.sipfoundry.sipxconfig.acd.AcdAgent;
import org.sipfoundry.sipxconfig.acd.AcdLine;
import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroup;
import org.sipfoundry.sipxconfig.admin.dialplan.AttendantRule;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.CustomDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRule;
import org.sipfoundry.sipxconfig.admin.dialplan.InternalRule;
import org.sipfoundry.sipxconfig.admin.dialplan.InternationalRule;
import org.sipfoundry.sipxconfig.admin.dialplan.LocalRule;
import org.sipfoundry.sipxconfig.admin.dialplan.LongDistanceRule;
import org.sipfoundry.sipxconfig.admin.dialplan.SiteToSiteDialingRule;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.Bridge;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.site.acd.EditAcdAgent;
import org.sipfoundry.sipxconfig.site.acd.EditAcdLine;
import org.sipfoundry.sipxconfig.site.acd.EditAcdQueue;
import org.sipfoundry.sipxconfig.site.admin.EditCallGroup;
import org.sipfoundry.sipxconfig.site.admin.EditParkOrbit;
import org.sipfoundry.sipxconfig.site.branch.EditBranch;
import org.sipfoundry.sipxconfig.site.conference.EditBridge;
import org.sipfoundry.sipxconfig.site.conference.EditConference;
import org.sipfoundry.sipxconfig.site.dialplan.EditAutoAttendant;
import org.sipfoundry.sipxconfig.site.gateway.EditGateway;
import org.sipfoundry.sipxconfig.site.phone.EditPhone;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;
import org.sipfoundry.sipxconfig.site.upload.EditUpload;
import org.sipfoundry.sipxconfig.site.user.EditUser;
import org.sipfoundry.sipxconfig.upload.Upload;

/**
 * This is a class in charge of determining which "edit" page should be used for an entity object
 * (bean). It does not seem to be any elegant way of solving this problem - we have object type
 * and id and we need to find an edit page for it.
 *
 */
public class EnumEditPageProvider implements EditPageProvider {
    public static final Log LOG = LogFactory.getLog(EnumEditPageProvider.class);

    public static final String RULE_ID = "ruleId";

    public static final Object[] PAGES = {
        User.class, new String[] {
            EditUser.PAGE, "userId"
        }, Group.class, new String[] {
            EditGroup.PAGE, "groupId"
        }, Phone.class, new String[] {
            EditPhone.PAGE, "phoneId"
        }, Gateway.class, new String[] {
            EditGateway.PAGE, "gatewayId"
        }, CallGroup.class, new String[] {
            EditCallGroup.PAGE, "callGroupId"
        }, Bridge.class, new String[] {
            EditBridge.PAGE, "bridgeId"
        }, Conference.class, new String[] {
            EditConference.PAGE, "conferenceId"
        }, ParkOrbit.class, new String[] {
            EditParkOrbit.PAGE, "parkOrbitId"
        }, AutoAttendant.class, new String[] {
            EditAutoAttendant.PAGE, "autoAttendantId"
        }, InternalRule.class, new String[] {
            "dialplan/EditInternalDialRule", RULE_ID
        }, CustomDialingRule.class, new String[] {
            "dialplan/EditCustomDialRule", RULE_ID
        }, LocalRule.class, new String[] {
            "dialplan/EditLocalDialRule", RULE_ID
        }, LongDistanceRule.class, new String[] {
            "dialplan/EditLongDistanceDialRule", RULE_ID
        }, EmergencyRule.class, new String[] {
            "dialplan/EditEmergencyDialRule", RULE_ID
        }, InternationalRule.class, new String[] {
            "dialplan/EditInternationalDialRule", RULE_ID
        }, AttendantRule.class, new String[] {
            "dialplan/EditAttendantDialRule", RULE_ID
        }, SiteToSiteDialingRule.class, new String[] {
            "dialplan/EditSiteToSiteDialRule", RULE_ID
        }, Upload.class, new String[] {
            EditUpload.PAGE, "uploadId"
        }, AcdServer.class, new String[] {
            "acd/AcdServerPage", "acdServerId"
        }, AcdQueue.class, new String[] {
            EditAcdQueue.PAGE, "acdQueueId"
        }, AcdLine.class, new String[] {
            EditAcdLine.PAGE, "acdLineId"
        }, AcdAgent.class, new String[] {
            EditAcdAgent.PAGE, "acdAgentId"
        }, Branch.class, new String[] {
            EditBranch.PAGE, "branchId"
        }
    };

    private final Map m_classToPageInfo;

    public EnumEditPageProvider() {
        m_classToPageInfo = new HashMap(PAGES.length / 2);
        for (int i = 0; i < PAGES.length; i = i + 2) {
            Class klass = (Class) PAGES[i];
            m_classToPageInfo.put(klass, PAGES[i + 1]);
        }
    }

    /**
     * This is used only in unit tests. We are making sure that all the pages that we reference
     * are actually available and that they have settebale "id" field.
     */
    public void validatePages(IRequestCycle cycle) {
        for (Iterator i = m_classToPageInfo.values().iterator(); i.hasNext();) {
            String[] pageInfo = (String[]) i.next();
            getEditPage(cycle, null, pageInfo, false);
        }
    }

    public IPage getPage(IRequestCycle cycle, Class klass, Object id) {
        for (Class k = klass; k != Object.class; k = k.getSuperclass()) {
            String[] pageInfo = (String[]) m_classToPageInfo.get(k);
            if (pageInfo != null) {
                return getEditPage(cycle, id, pageInfo, true);
            }
        }
        return null;
    }

    private IPage getEditPage(IRequestCycle cycle, Object id, String[] pageInfo, boolean ignoreExceptions) {
        Exception exception = null;
        try {
            IPage page = cycle.getPage(pageInfo[0]);
            // HACK: see http://issues.apache.org/bugzilla/show_bug.cgi?id=16525
            // we need to use copyProperty and not setProperty
            BeanUtils.copyProperty(page, pageInfo[1], id);
            return page;
        } catch (IllegalAccessException e) {
            exception = e;
        } catch (InvocationTargetException e) {
            exception = e;
        }
        if (!ignoreExceptions) {
            throw new RuntimeException(exception);
        }
        // if silent we only log it
        LOG.error(exception);
        return null;
    }
}
