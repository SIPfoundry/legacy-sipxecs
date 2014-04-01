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

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.easymock.classextension.EasyMock.createMock;
import static org.sipfoundry.sipxconfig.security.UserRole.Admin;
import static org.sipfoundry.sipxconfig.security.UserRole.User;

import java.util.ArrayList;
import java.util.Collection;
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
import org.sipfoundry.commons.userdb.profile.UserProfileService;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.CoreContextImpl;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.feature.FeatureManagerImpl;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.sipfoundry.sipxconfig.site.ApplicationLifecycle;
import org.sipfoundry.sipxconfig.site.ApplicationLifecycleImpl;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.skin.SkinControl;
import org.sipfoundry.sipxconfig.site.user.FirstUser;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.GrantedAuthority;

public class BorderTest extends TestCase {
    IPage m_dummyPage;
    IPage m_firstUserPage;
    IRequestCycle m_requestCycle;

    protected void setUp() throws Exception {
        m_dummyPage = createMock(IPage.class);
        m_firstUserPage = createMock(IPage.class);
        m_requestCycle = createMock(IRequestCycle.class);

        m_dummyPage.getRequestCycle();
        expectLastCall().andReturn(m_requestCycle);
        m_requestCycle.getPage(FirstUser.PAGE);
        expectLastCall().andReturn(m_firstUserPage);
        replay(m_dummyPage, m_requestCycle);
    }

    // Does not work if any other test has run before it
    public void DISABLED_testLogin() {
        Border restricted = new MockBorder(true, true, new UserSession());
        restricted.setPage(m_dummyPage);

        try {
            restricted.pageValidate(new PageEvent(m_dummyPage, null));
            fail("should redirect");
        } catch (PageRedirectException e) {
            assertEquals("LoginPage", e.getTargetPageName());
        }

    }

    public void testLoginNotRequired() {
        Border nologin = new MockBorder(true, false, new UserSession());
        nologin.setPage(m_dummyPage);
        nologin.pageValidate(null);
    }

    public void testRestricted() {
        Border restricted = new MockBorder(true, true, new MockUserSession(false));
        restricted.setPage(m_dummyPage);
        try {
            restricted.pageValidate(null);
            fail("should redirect to login page");
        } catch (PageRedirectException e) {
            assertEquals("vm/ManageVoicemail", e.getTargetPageName());
        }
    }

    public void testRestrictedAdmin() {
        Border restricted = new MockBorder(true, true, new MockUserSession(true));
        restricted.setPage(m_dummyPage);
        try {
            restricted.pageValidate(null);
        } catch (PageRedirectException e) {
            fail("unexpected expected");
        }
    }

    public void testUnrestricted() {
        Border unrestricted = new MockBorder(false, true, new MockUserSession(false));
        unrestricted.setPage(m_dummyPage);
        try {
            unrestricted.pageValidate(null);
        } catch (PageRedirectException e) {
            fail("unexpected expected");
        }
    }

    public void testUnrestrictedAdmin() {
        Border unrestricted = new MockBorder(false, true, new MockUserSession(true));
        unrestricted.setPage(m_dummyPage);
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

    public void testGetBorderTitle() {
        IPage page = createMock(IPage.class);
        Border border = new MockBorder(false, true, new MockUserSession(true));
        border.setPage(page);

        assertEquals("BorderTitle", border.getBorderTitle());
    }

    private static class MockUserSession extends UserSession {
        private final UserDetailsImpl m_userDetailsImpl;

        MockUserSession(boolean admin) {
            User user = new User();
            PermissionManager pManager = createMock(PermissionManager.class);
            pManager.getPermissionModel();
            expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
            replay(pManager);
            user.setPermissionManager(pManager);

            Collection<GrantedAuthority> authorities = new ArrayList<GrantedAuthority>();
            authorities.add(User.toAuth());
            if (admin) {
                authorities.add(Admin.toAuth());
                m_userDetailsImpl = new UserDetailsImpl(user, "bongo", authorities, true);
            } else {
                m_userDetailsImpl = new UserDetailsImpl(user, "bongo", authorities, false);
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

        @Override
        public String getBorderTitle() {
            return "BorderTitle";
        }

        public ICallback getLoginCallback() {
            return null;
        }

        @Override
        public ApplicationLifecycle getApplicationLifecycle() {
            return new ApplicationLifecycleImpl();
        }

        @Override
        public WebRequest getRequest() {
            return EasyMock.createNiceControl().createMock(WebRequest.class);
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

                @Override
                public UserProfileService getUserProfileService() {
                    return null;
                }

            };
        }

        @Override
        public FeatureManager getFeatureManager() {
            return new FeatureManagerImpl() {
                public boolean isFeatureEnabled(LocationFeature feature) {
                    return true;
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
        public String getHelpLink() {
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

        @Override
        public String getInitialSessionId() {
            return null;
        }

        @Override
        public void setInitialSessionId(String sessionId) {
        }

        @Override
        public boolean getHeaderDisplay() {
            return false;
        }

        @Override
        public boolean getFooterDisplay() {
            return false;
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
