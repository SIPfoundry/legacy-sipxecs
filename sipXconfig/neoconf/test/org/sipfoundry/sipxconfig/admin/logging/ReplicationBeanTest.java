package org.sipfoundry.sipxconfig.admin.logging;

import static org.easymock.EasyMock.createMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

import junit.framework.TestCase;

public class ReplicationBeanTest extends TestCase {
    private ReplicationBean m_replicationBean;
    private String m_fqdnMaster;
    private String m_fqdnSlave1;
    private String m_fqdnSlave2;

    protected void setUp() throws Exception {
        m_replicationBean = new ReplicationBean();
        m_replicationBean.setPath(TestHelper.getTestDirectory());
        m_fqdnMaster = "test.example.com";
        m_fqdnSlave1 = "test.slave1.com";
        m_fqdnSlave2 = "test.slave2.com";
        m_replicationBean.removeFlag(m_fqdnMaster);
        m_replicationBean.removeFlag(m_fqdnSlave1);
        m_replicationBean.removeFlag(m_fqdnSlave2);
    }

    public void testCreateRemoveFailedFlag() {
        ApplicationContext context = createMock(ApplicationContext.class);
        m_replicationBean.setApplicationContext(context);

        m_replicationBean.createFlagFailed(m_fqdnMaster);
        m_replicationBean.createFlagFailed(m_fqdnSlave1);
        assertTrue(m_replicationBean.isFailed(m_fqdnMaster));
        assertTrue(m_replicationBean.isFailed(m_fqdnSlave1));
        assertFalse(m_replicationBean.isFailed(m_fqdnSlave2));

        m_replicationBean.removeFlag(m_fqdnMaster);
        m_replicationBean.removeFlag(m_fqdnSlave1);
        m_replicationBean.createFlagFailed(m_fqdnSlave2);

        assertFalse(m_replicationBean.isFailed(m_fqdnMaster));
        assertFalse(m_replicationBean.isFailed(m_fqdnSlave1));
        assertTrue(m_replicationBean.isFailed(m_fqdnSlave2));

        m_replicationBean.createFlagFailed(m_fqdnMaster);
        m_replicationBean.createFlagFailed(m_fqdnSlave1);
        m_replicationBean.createFlagFailed(m_fqdnSlave2);

        assertTrue(m_replicationBean.isFailed(m_fqdnMaster));
        assertTrue(m_replicationBean.isFailed(m_fqdnSlave1));
        assertTrue(m_replicationBean.isFailed(m_fqdnSlave2));
    }

}
