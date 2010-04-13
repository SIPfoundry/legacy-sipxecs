require 'test/unit'
require 'ConfigServiceDriver.rb'
require 'ApiTestCase.rb'

class PhoneServiceTestApi < ApiTestCase
  
  def setup
    super
  
    # create the services that we will use
    @phone_service = service(PhoneService)
    @user_service = service(UserService)
    @test_service = service(TestService)

    reset = ResetServices.new();
    reset.phone = true;
    reset.user = true;
    @test_service.resetServices(reset)
  end
  
  def test_createPhone	    
    expected = Phone.new('000000000000', 'acmePhoneStandard')
    expected.groups = [ 'group1' ]
    @phone_service.addPhone(AddPhone.new(expected))
    
    findPhone = FindPhone.new(PhoneSearch.new(expected.serialNumber))	  
    phones = @phone_service.findPhone(findPhone).phones
    
    assert_equal(1, phones.length)
    assert_equal(expected.serialNumber, phones[0].serialNumber)	    
    assert_equal(1, phones[0].groups.length)
    assert_equal('group1', phones[0].groups[0])
  end
  
  def test_deletePhone
    seedPhone()
    
    # ensure phone was created
    findPhone = FindPhone.new(PhoneSearch.new())	  
    findPhone.search.bySerialNumber = @seed.serialNumber
    assert_equal(1, @phone_service.findPhone(findPhone).phones.length)
    
    deletePhone = ManagePhone.new(PhoneSearch.new())
    deletePhone.search.bySerialNumber = @seed.serialNumber
    deletePhone.deletePhone = true
    @phone_service.managePhone(deletePhone)
    
    # ensure phone was deleted
    assert_equal(0, @phone_service.findPhone(findPhone).phones.length)
  end
  
  def test_addRemoveGroup	
    seedPhone()
    
    search = PhoneSearch.new(@seed.serialNumber)
    
    #add group
    addGroup = ManagePhone.new(search)
    addGroup.addGroup = 'group1'
    @phone_service.managePhone(addGroup)
    phones = @phone_service.findPhone(FindPhone.new(search)).phones;
    assert_equal('group1', phones[0].groups[0])
    
    #remove group
    removeGroup = ManagePhone.new(search)
    removeGroup.removeGroup = 'group1'
    @phone_service.managePhone(removeGroup)
    phones = @phone_service.findPhone(FindPhone.new(search)).phones;
    assert_nil(phones[0].groups)	    
  end
  
  def test_addRemoveLine
    seedPhone()
    addUser1 = AddUser.new(User.new('user1'), '1234')
    @user_service.addUser(addUser1)
    
    # add line
    search = PhoneSearch.new(@seed.serialNumber)
    addLine = ManagePhone.new(search);
    addLine.addLine = Line.new('user1')
    @phone_service.managePhone(addLine)
    
    phone = @phone_service.findPhone(FindPhone.new(search)).phones[0]
    assert_equal(phone.lines[0].userId, 'user1')
    
    #remove line		
    removeLine = ManagePhone.new(search);
    removeLine.removeLineByUserId = 'user1'
    @phone_service.managePhone(removeLine)
    
    phone = @phone_service.findPhone(FindPhone.new(search)).phones[0]
    assert_nil(phone.lines)
  end
  
  def test_phoneEdit
    seedPhone()
    
    search = PhoneSearch.new(@seed.serialNumber)
    edit = ManagePhone.new(search);
    edit.edit = [ Property.new('description', 'hello') ]
    @phone_service.managePhone(edit)
    
    phone = @phone_service.findPhone(FindPhone.new(search)).phones[0]
    assert_equal(phone.description, 'hello')       
  end
  
  def test_phoneProfileAndRestart
    addUser1 = AddUser.new(User.new('user1'), '1234')
    @user_service.addUser(addUser1)
    
    @seed = Phone.new('000000000000', 'polycom300')
    addPhone = AddPhone.new(@seed)
    @seed.lines = [ Line.new('user1') ]
    @phone_service.addPhone(addPhone)
    
    generateProfiles = ManagePhone.new()
    generateProfiles.generateProfiles = true
    @phone_service.managePhone(generateProfiles)
    # this just excersizes code. does not verify profiles were generated
    
    restart = ManagePhone.new()
    restart.restart = true
    @phone_service.managePhone(restart)
    # this just excersizes code. does not verify restart message was sent	
  end
  
  def test_phoneDeviceVersion
    @seed = Phone.new('000000000000', 'polycom300')
    @seed.deviceVersion = 'polycom1.6'
    addPhone = AddPhone.new(@seed)
    @phone_service.addPhone(addPhone)
    
    search = PhoneSearch.new(@seed.serialNumber)
    phone = @phone_service.findPhone(FindPhone.new(search)).phones[0]
    assert_equal('polycom1.6', phone.deviceVersion)
  end
  
  def test_phoneSearch
    seedPhone()
    all = @phone_service.findPhone(FindPhone.new()).phones
    assert_equal(1, all.length)
    assert_equal(@seed.serialNumber, all[0].serialNumber)        
  end
  
  def test_addRemoveExternalLine
    seedPhone()    
    search = PhoneSearch.new(@seed.serialNumber)
    addExternalLine = ManagePhone.new(search);
    addExternalLine.addExternalLine = AddExternalLine.new('external_user', 'External User', '1234',
       'fwd.com', '101')
    @phone_service.managePhone(addExternalLine)
    
    #ERROR Nil in response.  Username is not persisting
    phone = @phone_service.findPhone(FindPhone.new(search)).phones[0]
    
    expectedUri = '"External User"<sip:external_user@fwd.com>'
    assert_equal(phone.lines[0].userId, 'external_user')
    assert_equal(phone.lines[0].uri, expectedUri)
    
    #remove line		
    removeLine = ManagePhone.new(search);
    removeLine.removeLineByUri = expectedUri
    @phone_service.managePhone(removeLine)
    
    phone = @phone_service.findPhone(FindPhone.new(search)).phones[0]
    assert_nil(phone.lines)
  end

  def seedPhone
    @seed = Phone.new('000000000000', 'acmePhoneStandard')
    addPhone = AddPhone.new(@seed)
    @phone_service.addPhone(addPhone)
  end
  
end