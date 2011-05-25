$:.unshift File.join(File.dirname(__FILE__), "..")
require 'test_settings'

$:.unshift File.join(File.dirname(__FILE__), "..", "lib")
require 'report'

require 'time'
require 'test/unit'

class  DaoTest < Test::Unit::TestCase

  @@dbi = nil

  def test_insert_sql
    actual = Reports::Dao.insert_sql('bird', ['dove', 'hawk'])    
    assert_equal("INSERT into bird (dove,hawk) VALUES (?,?)", actual)
  end
  
  def test_columns    
    birds = Reports::Dao.new('bird', DaoTest.connect)
    assert_equal ['species'], birds.columns
    DaoTest::disconnect
  end
  
  def DaoTest.connect
    if @@dbi == nil
      @@dbi = DBI.connect(DB_URI, DB_USER)
    end
    @@dbi
  end
  
  def DaoTest.disconnect
    @@dbi.disconnect if @@dbi != nil
    @@dbi = nil
  end
  
  def DaoTest.clear(table)
    DaoTest.connect.do("delete from #{table}")
  end
end

class CallStatDaoTest < Test::Unit::TestCase

  def test_get_param
      stat = CallAudit.new("from_me")
      dao = Reports::CallStatDao.new(DaoTest::connect)
      assert_equal "from_me", dao.get_param(stat, "from_uri")
      DaoTest::disconnect
  end

  def test_persist
    stats = []
    now = Time.now
    for i in 0...2
      stats << CallAudit.new("from#{i}", "state#{i}", "agent_uri#{i}", 
        "queue_uri#{i}", now, now + 1, now + 2)
    end
    dao = Reports::CallStatDao.new(DaoTest::connect)
    dao.persist(stats)    
    DaoTest::disconnect    
  end
  
  def test_load_window
    dbi = DummyDbi.new
    dao = Reports::CallStatDao.new(dbi)
    DaoTest::clear(dao.table)
    dao.load_window

    obj = CallAudit.new("joe")
    obj.terminate_time = dbi.t.to_utc_datetime
    assert ! dao.valid?(obj)

    obj = CallAudit.new("mary")
    obj.terminate_time = dbi.t.to_utc_datetime

    assert ! dao.valid?(obj)

    obj = CallAudit.new("joe")
    obj.terminate_time = dbi.t.to_utc_datetime + 1
    assert dao.valid?(obj)

    obj = CallAudit.new("joey")
    obj.terminate_time = dbi.t.to_utc_datetime
    assert dao.valid?(obj)
    
    DaoTest::disconnect
  end
  
  def test_load_columns
    dao = Reports::CallStatDao.new(DaoTest::connect)
    columns = dao.load_columns
    assert columns.include?('enter_time')
    DaoTest::disconnect
  end
  
end

class DummyDbi

  attr_accessor :t
    
  def initialize()
    self.t = DBI::Timestamp.new(Time.now)
  end
     
  def select_all(sql)
    yield ["joe", self.t]
    yield ["mary", self.t]
  end
end

class AgentStatDaoTest < Test::Unit::TestCase
  def test_persist
    stats = []
    now = DateTime.now
    for i in 0...2
      stats << AgentAudit.new("agent_uri#{i}", "queue_uri#{i}", 
        now, now)
    end
    dao = Reports::AgentStatDao.new(DaoTest::connect)
    dao.persist(stats)    
    DaoTest::disconnect    
  end

  def test_load_window
    dbi = DummyDbi.new
    dao = Reports::AgentStatDao.new(dbi)
    DaoTest::clear(dao.table)
    dao.load_window

    obj = AgentAudit.new("joe")
    obj.sign_out_time = dbi.t.to_utc_datetime 
    assert ! dao.valid?(obj)

    obj = AgentAudit.new("mary")
    obj.sign_out_time = dbi.t.to_utc_datetime
    assert ! dao.valid?(obj)

    obj = AgentAudit.new("joe")
    obj.sign_out_time = dbi.t.to_utc_datetime + 1
    assert dao.valid?(obj)

    obj = AgentAudit.new("joey")
    obj.sign_out_time = dbi.t.to_utc_datetime
    assert dao.valid?(obj)
    
    DaoTest::disconnect
  end
end
