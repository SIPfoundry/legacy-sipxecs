package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.sipxconfig.commserver.imdb.MongoTestCaseHelper.assertObjectWithIdFieldValuePresent;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.acccode.AuthCode;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.test.ImdbTestCase;

public class AuthCodesTestIntegration extends ImdbTestCase {
    private final String[][] AUTHCODES_DATA = {
        {
            "0", "333", "~ac~1", "DMAAw3q6e6"
        }, {
            "1", "121", ",~ac~2", "DFDS78sS"
        }, {
            "2", "556", ",~ac~3", "QWE12Csd"
        },
    };

    private final String[][] INTERNAL_USER_DATA = {
        {
            "0", "~~ac~1", "DMAAw3q6e6"
        }, {
            "1", "~~ac~2", ",DFDS78sS"
        }, {
            "2", "~~ac~3", ",QWE12Csd"
        },
    };

    private List<AuthCode> m_authCodes;
    private List<InternalUser> m_internalUsers;

    public void testReplicateAuthCode() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        m_authCodes = new ArrayList<AuthCode>();
        m_internalUsers = new ArrayList<InternalUser>();
        InternalUser user = null;
        for (String[] internalUser : INTERNAL_USER_DATA) {
            user = new InternalUser();
            user.setPermissionManager(getPermissionManager());
            user.setUniqueId(new Integer(internalUser[0]));
            user.setName(internalUser[1]);
            user.setSipPassword(internalUser[2]);
            user.setDomainManager(getDomainManager());
            m_internalUsers.add(user);
        }
        Iterator<InternalUser> iterator = m_internalUsers.iterator();
        AuthCode authCode = null;
        for (String[] code : AUTHCODES_DATA) {
            authCode = new AuthCode();
            authCode.setUniqueId(new Integer(code[0]));
            authCode.setCode(code[1]);
            authCode.setName(code[2]);
            authCode.setDescription(code[3]);
            authCode.setInternalUser(iterator.next());
            m_authCodes.add(authCode);
        }
        getReplicationManager().replicateEntity(m_authCodes.get(0));
        getReplicationManager().replicateEntity(m_authCodes.get(1));
        getReplicationManager().replicateEntity(m_authCodes.get(2));
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "AuthCode0", MongoConstants.AUTH_CODE, AUTHCODES_DATA[0][1]);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "AuthCode1", MongoConstants.AUTH_CODE, AUTHCODES_DATA[1][1]);
        assertObjectWithIdFieldValuePresent(getEntityCollection(), "AuthCode2", MongoConstants.AUTH_CODE, AUTHCODES_DATA[2][1]);
    }
}
