/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.admin.dialplan.CallTag;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.sipfoundry.sipxconfig.site.UserSession;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class CdrTable extends BaseComponent {

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @Parameter
    public abstract Object getSource();

    public abstract Cdr getRow();

    public ITableColumn getStartTimeColumn() {
        return TapestryUtils.createDateColumn("startTime", getMessages(), getExpressionEvaluator(), getPage()
                .getLocale());
    }

    @InjectObject("spring:domainManager")
    public abstract DomainManager getDomainManager();

    @InjectObject("spring:sip")
    public abstract SipService getSipService();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @Asset("/images/user.png")
    public abstract IAsset getNormalUserIcon();

    @Asset("/images/email.png")
    public abstract IAsset getVoiceMailIcon();

    @Asset("/images/park.png")
    public abstract IAsset getParkIcon();

    @Asset("/images/retrievepark.png")
    public abstract IAsset getRetrieveParkIcon();

    /**
     * Implements click to call link
     *
     * @param number number to call - refer is sent to current user
     */
    public void call(String number) {
        String domain = getDomainManager().getDomain().getName();
        String userAddrSpec = getUser().getAddrSpec(domain);
        String destAddrSpec = SipUri.fix(number, domain);
        String displayName = "ClickToCall";
        getSipService().sendRefer(getUser(), userAddrSpec, displayName, destAddrSpec);
    }

    public User getUser() {
        return getUserSession().getUser(getCoreContext());
    }

    public IAsset getRecipientTypeIcon() {
        IAsset recipientIcon = null;
        String rowCalleeRoute = null;
        rowCalleeRoute = getRow().getCalleeRouteTail();
        if ((rowCalleeRoute != null) && (rowCalleeRoute.length() > 0)) {
            CallTag calltag = CallTag.valueOf(rowCalleeRoute);
            switch (calltag) {
            case VM:
                recipientIcon = getVoiceMailIcon();
                break;
            case RPARK:
                recipientIcon = getRetrieveParkIcon();
                break;
            case PARK:
                recipientIcon = getParkIcon();
                break;
            default:
                recipientIcon = null;
            }
        }
        return recipientIcon;
    }

    public String getRecipientIconTitle() {
        String rowCalleeRoute = null;
        rowCalleeRoute = getRow().getCalleeRouteTail();
        if ((rowCalleeRoute != null) && (rowCalleeRoute.length() > 0)) {
            String key = CallTag.valueOf(rowCalleeRoute).toString();
            return getMessages().getMessage(key);
        }
        return "None";
    }

}
