require 'test/unit'
require 'ConfigServiceDriver.rb'
require 'ApiTestCase.rb'

class UserServiceTestApi < ApiTestCase
  
  def setup
    super
  
    # create the services that we will use
    @user_service = service(UserService)
    @test_service = service(TestService)
    
    # reset the user service so we start the test with no users
    reset = ResetServices.new();
    reset.user = true;
    @test_service.resetServices(reset)
  end
  
  def test_createUser	    
    expected = User.new('lipton', '1234')    
    addUser = AddUser.new(expected)
    expected.groups = [ 'group1' ]
    @user_service.addUser(addUser)
    
    findUser = FindUser.new(UserSearch.new())
    findUser.search.byUserName = expected.userName
    users = @user_service.findUser(findUser).users    
    assert_equal(1, users.length)
    assert_equal(expected.userName, users[0].userName)		
  end
  
  def test_findUser
    user1 = User.new('joe')
    @user_service.addUser(AddUser.new(user1, '1234'))
    user2 = User.new('joe-boy')
    @user_service.addUser(AddUser.new(user2, '1234'))
    user3 = User.new('mary-joe')
    @user_service.addUser(AddUser.new(user3, '1234'))
    
    findUser = FindUser.new(UserSearch.new())
    findUser.search.byUserName = 'joe'
    users = @user_service.findUser(findUser).users
    assert_equal(1, users.length)
  end
  
  def test_deleteUser
    @user_service.addUser(AddUser.new(User.new('john'), '1234'))
    
    search = UserSearch.new();
    search.byUserName = 'john'
    users = @user_service.findUser(FindUser.new(search)).users
    assert_equal(1, users.length)
    
    deleteUser = ManageUser.new(search);
    deleteUser.deleteUser = true
    @user_service.manageUser(deleteUser)
    
    users = @user_service.findUser(FindUser.new(search)).users
    assert_equal(0, users.length)
  end
  
  def test_addGroup             
    addUser1 = AddUser.new(User.new('user1'), '1234')
    @user_service.addUser(addUser1)    
    
    search = UserSearch.new(addUser1.user.userName)
    users = @user_service.findUser(FindUser.new(search)).users
    assert_nil(users[0].groups)
    
    addGroup = ManageUser.new(search);
    addGroup.addGroup = 'yaya'
    @user_service.manageUser(addGroup)
    
    users = @user_service.findUser(FindUser.new(search)).users
    assert_equal(1, users[0].groups.length)
    assert_equal('yaya', users[0].groups[0])
  end
  
  def test_removeGroup	                      
    addUser1 = AddUser.new(User.new('user1'), '1234')
    addUser1.user.groups = [ 'group1', 'group2' ]
    @user_service.addUser(addUser1)    
    
    search = UserSearch.new(addUser1.user.userName)  
    removeGroup = ManageUser.new(search);
    removeGroup.removeGroup = 'group2'
    @user_service.manageUser(removeGroup)
    
    users = @user_service.findUser(FindUser.new(search)).users
    assert_equal(1, users[0].groups.length)
    assert_equal('group1', users[0].groups[0])
  end
  
  def test_editUser
    addUser1 = AddUser.new(User.new('user1'), '1234')
    addUser1.user.firstName = 'Ali'
    addUser1.user.lastName = 'Baba'
    addUser1.user.groups = [ 'group1', 'group2' ]
    @user_service.addUser(addUser1)
    
    addUser2 = AddUser.new(User.new('user2'), '1234')
    addUser2.user.firstName = 'Holy'
    addUser2.user.lastName = 'Mackerel'
    addUser2.user.groups = [ 'group1', 'group3' ]
    @user_service.addUser(addUser2)
    
    addUser3 = AddUser.new(User.new('user3'), '1234')
    addUser3.user.firstName = 'Sim'
    addUser3.user.lastName = 'Salabim'
    addUser3.user.groups = [ 'group2', 'group3' ]
    @user_service.addUser(addUser3)
    
    search = UserSearch.new()
    search.byGroup = 'group3'
    editUser = ManageUser.new(search)
    editUser.edit = [ 
    Property.new('firstName', 'Secret'), 
    Property.new('lastName', 'Agent') 
    ]
    users = @user_service.manageUser(editUser)
    
    users = @user_service.findUser(FindUser.new(nil)).users
    users.sort do |a, b|
      a.userName <=> b.userName
    end
    assert_equal('Ali', users[0].firstName)
    assert_equal('Baba', users[0].lastName)
    assert_equal('Secret', users[1].firstName)
    assert_equal('Agent', users[1].lastName)
    assert_equal('Secret', users[2].firstName)
    assert_equal('Agent', users[2].lastName)
  end
  
  def test_add_email
    expected = User.new('jo-jo', '1234')
    expected.emailAddress = 'jo-jo@example.com'
    addUser = AddUser.new(expected)
    @user_service.addUser(addUser)

    findUser = FindUser.new(UserSearch.new())
    findUser.search.byUserName = expected.userName
    users = @user_service.findUser(findUser).users
    assert_equal(1, users.length)
    assert_equal(expected.emailAddress, users[0].emailAddress)
  end
end