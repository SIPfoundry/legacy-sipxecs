package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.IM_ENABLED;
import static org.sipfoundry.commons.mongo.MongoConstants.PERMISSIONS;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.SortedSet;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.common.Closure;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialGroup;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;

import com.mongodb.BasicDBObject;
import com.mongodb.DBObject;

public class SpeedDialsTestIntegration extends ImdbTestCase {
    private SpeedDials m_speeddialDataSet;
    private SettingDao m_settingDao;
    private SpeedDialManager m_speedDialManager;

    public void testGenerateResourceLists() throws Exception {
        loadDataSetXml("domain/DomainSeed.xml");
        //loadDataSet("commserver/imdb/speeddials.db.xml");
        sql("commserver/imdb/speeddials.sql");
        
        /* TODO: enable test for speed dial groups; 
         * i don't understand why a group with 2 members when it gets to:
         *  DaoUtils.forAllGroupMembersDo(CoreContext coreContext, Group group,
            Closure<User> closure, int start, int pageSize)
            the return of  coreContext.getGroupMembersCount(group.getId()) is 0
        SpeedDialGroup spdlGroup = m_speedDialManager.getSpeedDialForGroupId(1001);
        m_speedDialManager.saveSpeedDialGroup(spdlGroup);*/
        

        //just to trigger the replication
        User userA = getCoreContext().loadUserByUserName("user_a");
        User userB = getCoreContext().loadUserByUserName("user_b");
        User userC = getCoreContext().loadUserByUserName("user_c");
        User userD = getCoreContext().loadUserByUserName("user_d");
        getCoreContext().saveUser(userA);
        getCoreContext().saveUser(userB);
        getCoreContext().saveUser(userC);
        getCoreContext().saveUser(userD);
        
        DBObject user1 = new BasicDBObject().append(ID, "User9991").append(UID, "user_a");
        BasicDBObject speeddial1 = new BasicDBObject("usr", "~~rl~F~user_a")
            .append("usrcns", "~~rl~C~user_a");
        List<DBObject> btns1 = new ArrayList<DBObject>();
        btns1.add(new BasicDBObject("uri", "sip:102@example.org").append("name", "beta"));
        btns1.add(new BasicDBObject("uri", "sip:104@sipfoundry.org").append("name", "gamma"));
        speeddial1.append("btn", btns1);
        user1.put("spdl", speeddial1);
        
        DBObject user2 = new BasicDBObject().append(ID, "User9992").append(UID, "user_b");
        BasicDBObject speeddial2 = new BasicDBObject("usr", "~~rl~F~user_b")
            .append("usrcns", "~~rl~C~user_b");
        List<DBObject> btns2 = new ArrayList<DBObject>();
        btns2.add(new BasicDBObject("uri", "sip:404@example.org").append("name", "beta1"));
        speeddial2.append("btn", btns2);
        user2.put("spdl", speeddial2);
        
        
        DBObject user3 = new BasicDBObject().append(ID, "User9993").append(UID, "user_c");
        BasicDBObject speeddial3 = new BasicDBObject();
        user3.put("spdl", speeddial3);

        DBObject user4 = new BasicDBObject().append(ID, "User9994").append(UID, "user_d");
        BasicDBObject speeddial4 = new BasicDBObject("usr", "~~rl~F~user_d")
            .append("usrcns", "~~rl~C~user_d");
        List<DBObject> btns4 = new ArrayList<DBObject>();
        btns4.add(new BasicDBObject("uri", "sip:101@example.org").append("name", "alpha"));
        speeddial4.append("btn", btns4);
        user4.put("spdl", speeddial4);
        
        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user1);
        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user2);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent(getEntityCollection(), "User9993", "spdl", null);
        
        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user4);
        m_speedDialManager.deleteSpeedDialsForUser(9994);
        getCoreContext().saveUser(userD);
        MongoTestCaseHelper.assertObjectWithIdFieldValuePresent(getEntityCollection(), "User9994", "spdl", null);
    }

    public void testSpeedDials() {
        
    }
    
    public void setSpeeddialDataSet(SpeedDials speedDials) {
        m_speeddialDataSet = speedDials;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setSpeedDialManager(SpeedDialManager speedDialManager) {
        m_speedDialManager = speedDialManager;
    }
}
