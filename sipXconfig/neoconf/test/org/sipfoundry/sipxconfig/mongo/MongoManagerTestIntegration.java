package org.sipfoundry.sipxconfig.mongo;

import java.util.Collection;
import java.util.Map;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class MongoManagerTestIntegration extends IntegrationTestCase {
    private MongoManager m_mongoManager;    
    
    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testSaveShard() {
        Collection<MongoShard> shardsBefore = m_mongoManager.getShards();
        assertEquals(0, shardsBefore.size());
        MongoShard shard = new MongoShard();
        shard.setName("McGillicutty");
        m_mongoManager.saveShard(shard);
        Map<String, Object> actual = getJdbcTemplate().queryForMap("select * from shard");        
        assertEquals("McGillicutty", actual.get("name"));
        assertTrue(((Integer) actual.get("shard_id")) > 0);
    }

    public void setMongoManager(MongoManager mongoManager) {
        m_mongoManager = mongoManager;
    }
}
