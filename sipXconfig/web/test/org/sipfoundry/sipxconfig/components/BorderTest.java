/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.components;

import java.util.Locale;

import junit.framework.TestCase;
import org.apache.hivemind.Messages;
import org.apache.hivemind.impl.AbstractMessages;
import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRender;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.components.Block;
import org.apache.tapestry.engine.IEngineService;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.web.WebRequest;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.VersionInfo;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.sipfoundry.sipxconfig.site.ApplicationLifecycle;
import org.sipfoundry.sipxconfig.site.ApplicationLifecycleImpl;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.skin.SkinControl;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.sipfoundry.sipxconfig.security.UserRole.Admin;
import static org.sipfoundry.sipxconfig.security.UserRole.User;

public class BorderTest extends TestCase {

    public void testLogin() {
        IPage dummyPage = EasyMock.createNiceControl().createMock(IPage.class);
        Border restricted = new MockBorder(true, true, new UserSession());
        try {
            restricted.pageValidate(new PageEvent(dummyPage, null));
            fail("should redirect");
        } catch (PageRedirectException e) {
            assertEquals("LoginPage", e.getTargetPageName());
        }
    }

    public void testLoginNotRequired() {
        Border nologin = new MockBorder(true, false, new UserSession());
        nologin.pageValidate(null);
    }

    public void testRestricted() {
        Border restricted = new MockBorder(true, true, new MockUserSession(false));

        try {
            restricted.pageValidate(null);
            fail("should redirect to login page");
        } catch (PageRedirectException e) {
            assertEquals("Home", e.getTargetPageName());
        }
    }

    public void testRestrictedAdmin() {
        Border restricted = new MockBorder(true, true, new MockUserSession(true));

        try {
            restricted.pageValidate(null);
        } catch (PageRedirectException e) {
            fail("unexpected expected");
        }
    }

    public void testUnrestricted() {
        Border unrestricted = new MockBorder(false, true, new MockUserSession(false));

        try {
            unrestricted.pageValidate(null);
        } catch (PageRedirectException e) {
            fail("unexpected expected");
        }
    }

    public void testUnrestrictedAdmin() {
        Border unrestricted = new MockBorder(false, true, new MockUserSession(true));

        try {
            unrestricted.pageValidate(null);
        } catch (PageRedirectException e) {
            fail("unexpected expected");
        }
    }

    public void testGetPageTitle() {
        IPage page = createMock(IPage.class);
        expect(page.getMessages()).andReturn(new FullMessages());
        expect(page.getMessages()).andReturn(new EmptyMessages());
        replay(page);

        Border border = new MockBorder(false, true, new MockUserSession(true));
        border.setPage(page);

        assertEquals("Product::PageTitle", border.getPageTitle());
        assertEquals("Product", border.getPageTitle());

        verify(page);
    }

    private static class MockUserSession extends UserSession {
        private final UserDetailsImpl m_userDetailsImpl;

        MockUserSession(boolean admin) {
            User user = new User();
            if (admin) {
                m_userDetailsImpl = new UserDetailsImpl(user, "bongo", User.toAuth(), Admin.toAuth());
            } else {
                m_userDetailsImpl = new UserDetailsImpl(user, "bongo", User.toAuth());
            }
        }

        @Override
        protected UserDetailsImpl getUserDetails() {
            return m_userDetailsImpl;
        }
    }

    private static class MockBorder extends Border {
        private final boolean m_restricted;
        private final boolean m_loginRequired;
        private final UserSession m_userSession;
        private String m_clientId;

        MockBorder(boolean restricted, boolean loginRequired, UserSession userSession) {
            m_restricted = restricted;
            m_loginRequired = loginRequired;
            m_userSession = userSession;
        }

        @Override
        public void setNavigationBlock(Block block) {

        }

        @Override
        public Block getNavigationBlock() {
            return null;
        }

        @Override
        public boolean isLoginRequired() {
            return m_loginRequired;
        }

        @Override
        public boolean isRestricted() {
            return m_restricted;
        }

        @Override
        public UserSession getUserSession() {
            return m_userSession;
        }

        public ICallback getLoginCallback() {
            return null;
        }

        @Override
        public ApplicationLifecycle getApplicationLifecycle() {
            return new ApplicationLifecycleImpl();
        }

        @Override
        protected void redirectToLogin(IPage page, IRequestCycle cycle) {
            throw new PageRedirectException("LoginPage");
        }

        @Override
        public IEngineService getRestartService() {
            return null;
        }

        @Override
        public CoreContext getCoreContext() {
            return new CoreContextImpl() {
                @Override
                public int getUsersCount() {
                    return 1;
                }

                @Override
                public User newUser() {
                    return null;
                }

                @Override
                public InternalUser newInternalUser() {
                    return null;
                }

            };
        }

        public SkinControl getSkin() {
            return null;
        }

        @Override
        public String getClientId() {
            return m_clientId;
        }

        @Override
        public void setClientId(String id) {
            m_clientId = id;
        }

        @Override
        public String getBaseUrl() {
            return null;
        }

        @Override
        public String getHelpLink(Integer[] versionIds) {
            return null;
        }

        @Override
        public WebRequest getRequest() {
            return null;
        }

        @Override
        public TapestryContext getTapestry() {
            return null;
        }

        @Override
        public boolean getUseDojo() {
            return false;
        }

        @Override
        public VersionInfo getVersionInfo() {
            return null;
        }

        @Override
        public void setBaseUrl(String baseUrl) {
        }

        @Override
        public IRender getShellDelegate() {
            return null;
        }

        @Override
        public IAsset getBorderDateScript() {
            return null;
        }

        @Override
        public Messages getMessages() {
            return new FullMessages();
        }
    }

    private static class FullMessages extends AbstractMessages {
        @Override
        protected Locale getLocale() {
            return Locale.US;
        }

        @Override
        protected String findMessage(String key) {
            if (key.equals("product.name")) {
                return "Product";
            }
            if (key.equals("title")) {
                return "PageTitle";
            }
            return null;
        }
    }

    private static class EmptyMessages extends AbstractMessages {
        @Override
        protected Locale getLocale() {
            return Locale.US;
        }

        @Override
        protected String findMessage(String key) {
            return null;
        }
    }
}
