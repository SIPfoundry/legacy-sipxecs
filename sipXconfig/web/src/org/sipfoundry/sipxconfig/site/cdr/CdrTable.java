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

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.dialplan.CallTag;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.sip.SipService;
import org.sipfoundry.sipxconfig.site.UserSession;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class CdrTable extends BaseComponent {
    private static final String DOT = "\\.";
    private static final String IP_ADDR_OCTET = "(?:25[0-5]|2[0-4][0-9]|[01]?[0-9]{1,2})";
    private static final String IP_ADDR = IP_ADDR_OCTET + DOT + IP_ADDR_OCTET + DOT + IP_ADDR_OCTET + DOT
            + IP_ADDR_OCTET;
    private static final String PREFIX = "(?:\".*\" *)?";
    private static final String AOR = PREFIX + "<?(sip:\\w+@(?:(?:(?:(?:\\w+|\\w+-\\w+)\\.)+[A-Za-z]+)|" + IP_ADDR
            + "))>?";
    private static final Pattern AOR_RE = Pattern.compile(AOR);
    private static final Pattern FULL_USER_RE = Pattern.compile("(?:\\w+ *)+ - (\\d+)");

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

    @Asset("/images/page.png")
    public abstract IAsset getPageIcon();

    @Asset("/images/site2site.png")
    public abstract IAsset getSiteToSiteIcon();

    @Asset("/images/longdistance.png")
    public abstract IAsset getLongDistIcon();

    @Asset("/images/tollfree.png")
    public abstract IAsset getTollFreeIcon();

    @Asset("/images/callpickup.png")
    public abstract IAsset getPickupIcon();

    @Asset("/images/autoattendant.png")
    public abstract IAsset getAutoAttIcon();

    @Asset("/images/restricted.png")
    public abstract IAsset getRestrictedIcon();

    @Asset("/images/mobile.png")
    public abstract IAsset getMobileIcon();

    @Asset("/images/emergency.png")
    public abstract IAsset getEmergencyIcon();

    @Asset("/images/retrievepark.png")
    public abstract IAsset getRetrieveParkIcon();

    /**
     * Implements click to call link
     *
     * @param number number to call - refer is sent to current user
     */
    public void call(String number) {
        String extension = number;
        String domain = getDomainManager().getDomain().getName();
        String userAddrSpec = getUser().getAddrSpec(domain);
        Matcher matcher = FULL_USER_RE.matcher(extension);
        if (matcher.matches()) {
            extension = matcher.group(1);
        }
        String destAddrSpec = SipUri.fix(extension, domain);
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
            case PAGE:
                recipientIcon = getPageIcon();
                break;
            case STS:
                recipientIcon = getSiteToSiteIcon();
                break;
            case LD:
                recipientIcon = getLongDistIcon();
                break;
            case TF:
                recipientIcon = getTollFreeIcon();
                break;
            case DPUP:
                recipientIcon = getPickupIcon();
                break;
            case AA:
                recipientIcon = getAutoAttIcon();
                break;
            case REST:
                recipientIcon = getRestrictedIcon();
                break;
            case MOB:
                recipientIcon = getMobileIcon();
                break;
            case EMERG:
                recipientIcon = getEmergencyIcon();
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

    public boolean isSelfCallCalleeCondition() {
        if (!getUserSession().isAdmin()) {
            String calleeAor = getRow().getCalleeAor();
            String userUri = getUser().getUri(getDomainManager().getDomain().getName());
            return isSelfCallCondition(calleeAor, userUri);
        }
        return true;
    }

    public boolean isSelfCallCallerCondition() {
        if (!getUserSession().isAdmin()) {
            String callerAor = getRow().getCallerAor();
            String userUri = getUser().getUri(getDomainManager().getDomain().getName());
            return isSelfCallCondition(callerAor, userUri);
        }
        return true;
    }

    public boolean isSelfCallCondition(String calleeOrCallerAor, String userUri) {
        String callAor = calleeOrCallerAor;
        String userAor = userUri;
        Matcher matcher = AOR_RE.matcher(calleeOrCallerAor);
        if (matcher.matches()) {
            callAor = matcher.group(1);
        }
        matcher = AOR_RE.matcher(userUri);
        if (matcher.matches()) {
            userAor = matcher.group(1);
        }
        if (userAor != null && callAor != null && userAor.equals(callAor)) {
            return true;
        }
        return false;
    }
}
