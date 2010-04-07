/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import java.text.DateFormat;
import java.util.Collection;
import java.util.Date;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IExternalPage;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRender;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.Tapestry;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.callback.ExternalCallback;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.engine.ILink;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageValidateListener;
import org.apache.tapestry.link.StaticLink;
import org.apache.tapestry.web.WebRequest;
import org.apache.tapestry.web.WebSession;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.VersionInfo;
import org.sipfoundry.sipxconfig.site.ApplicationLifecycle;
import org.sipfoundry.sipxconfig.site.Home;
import org.sipfoundry.sipxconfig.site.LoginPage;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.common.LeftNavigation;
import org.sipfoundry.sipxconfig.site.user.FirstUser;
import org.sipfoundry.sipxconfig.site.vm.ManageVoicemail;

@ComponentClass(allowInformalParameters = false)
public abstract class Border extends BaseComponent implements PageValidateListener, PageBeginRenderListener {

    @InjectObject(value = "spring:versionInfo")
    public abstract VersionInfo getVersionInfo();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectObject(value = "infrastructure:request")
    public abstract WebRequest getRequest();

    @InjectObject(value = "service:sipxconfig.ApplicationLifecycle")
    public abstract ApplicationLifecycle getApplicationLifecycle();

    @InjectObject(value = "engine-service:restart")
    public abstract IEngineService getRestartService();

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @Asset("context:/WEB-INF/common/BorderDate.script")
    public abstract IAsset getBorderDateScript();

    /**
     * When true - page does not require login
     */
    @Parameter(defaultValue = "true")
    public abstract boolean isLoginRequired();

    /**
     * When true - only SUPER can see the pages, when false END_USER is accepted as well as admin
     */
    @Parameter(defaultValue = "true")
    public abstract boolean isRestricted();

    @Deprecated
    @Parameter(defaultValue = "false")
    public abstract boolean getUseDojo();

    @Parameter
    public abstract IRender getShellDelegate();

    /**
     * Gets the {@code Block} being used as the navigation area.
     *
     * @return the {@code Block} that will be rendered in the navigation area.
     */
    public abstract Block getNavigationBlock();

    /**
     * Sets a {@code Block} component to be used as the left navigation block.
     *
     * @param navigationBlock The {@code Block} to render in the navigation area.
     */
    public abstract void setNavigationBlock(Block navigationBlock);

    public abstract String getBaseUrl();

    public abstract void setBaseUrl(String baseUrl);

    @Persist(value = "client")
    public abstract String getInitialSessionId();

    public abstract void setInitialSessionId(String sessionId);

    @Parameter(required = true)
    public abstract String getBorderTitle();

    public void pageBeginRender(PageEvent event) {
        if (getBaseUrl() == null) {
            String baseUrl = getRequest().getContextPath();
            setBaseUrl(baseUrl);
        }

    }

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        if (getNavigationBlock() == null) {
            Collection<IComponent> topLevelComponents = getPage().getComponents().values();
            Block navigationBlock = searchForNavigationBlock(topLevelComponents);
            setNavigationBlock(navigationBlock);
        }
    }

    public String getDate() {
        DateFormat dateFormat = DateFormat.getDateTimeInstance();
        return dateFormat.format(new Date());
    }

    // FIXME: change to search by name - it's kept in the hashmap by name
    private Block searchForNavigationBlock(Collection<IComponent> components) {
        for (IComponent value : components) {
            if (value instanceof LeftNavigation) {
                return (LeftNavigation) value;
            }
        }
        for (IComponent value : components) {
            Block navigationBlock = searchForNavigationBlock(value.getComponents().values());
            if (navigationBlock != null) {
                return navigationBlock;
            }
        }

        return null;
    }

    public void pageValidate(PageEvent event) {

        if (!isLoginRequired()) {
            return;
        }

        // If there are no users, then we need to create the first user
        if (getCoreContext().getUsersCount() == 0) {
            throw new PageRedirectException(FirstUser.PAGE);
        }

        UserSession user = getUserSession();

        // XX-6132: prevent mixing session - store and check original session id against the
        // current one
        WebSession session = getRequest().getSession(false);
        if (session != null) {
            String initialSessionId = getInitialSessionId();
            String currentSessionId = session.getId();

            if (initialSessionId == null) {
                setInitialSessionId(currentSessionId);
            }

            if (!getInitialSessionId().equals(currentSessionId)) {
                if (user.isAdmin()) {
                    throw new PageRedirectException(Home.PAGE);
                } else {
                    throw new PageRedirectException(ManageVoicemail.PAGE);
                }
            }
        }

        // If there are users, but no one is logged in, then force a login
        if (!user.isLoggedIn()) {
            redirectToLogin(getPage(), event.getRequestCycle());
        }

        // If the logged-in user is not an admin, and this page is restricted, then
        // redirect the user to the home page since they are not worthy.
        // (We should probably use an error page instead of just tossing them home.)
        if (!user.isAdmin() && isRestricted()) {
            throw new PageRedirectException(Home.PAGE);
        }
    }

    public ILink logout(IRequestCycle cycle) {
        getApplicationLifecycle().logout();
        return new StaticLink(cycle.getAbsoluteURL("/"));
    }

    protected void redirectToLogin(IPage page, IRequestCycle cycle) {
        LoginPage login = (LoginPage) page.getRequestCycle().getPage(LoginPage.PAGE);
        conditionallySetLoginCallback(login, page, cycle);
        throw new PageRedirectException(login);
    }

    private void conditionallySetLoginCallback(LoginPage login, IPage page, IRequestCycle cycle) {
        // If page service is calling page, you're probably logged out and user could be
        // looking at a very different page then what gets interpretted from this callback
        // because all session data is lost.
        //
        // If your page can safely handle this, consider refactoring border component to accept a
        // component parameter to circumvent this constraint. Warning: this means your page
        // either doesn't use session variabled OR can safely recover when session is lost
        //
        String serviceName = cycle.getService().getName();
        if (Tapestry.EXTERNAL_SERVICE.equals(serviceName)) {
            ICallback callback = new ExternalCallback((IExternalPage) page, cycle.getListenerParameters());
            login.setCallback(callback);
        }
    }

    public IAsset[] getStylesheets() {
        return getTapestry().getStylesheets(this);
    }

    public String getLocaleValue() {
        return "locale=" + getPage().getLocale().getLanguage();
    }

    public String getHelpLink(Integer[] versionIds) {
        return getMessages().format("help.link", versionIds);
    }

    public String getUserName() {
        UserSession userSession = getUserSession();
        if (!userSession.isLoggedIn()) {
            return StringUtils.EMPTY;
        }
        User user = userSession.getUser(getCoreContext());
        return user.getLabel();
    }

    public String getPageTitle() {
        String productTitle = getMessages().getMessage("product.name");
        String pageTitle = LocalizationUtils.getMessage(getPage().getMessages(), "title", null);
        if (pageTitle == null) {
            return productTitle;
        }
        return String.format("%s::%s", productTitle, pageTitle);
    }
}
