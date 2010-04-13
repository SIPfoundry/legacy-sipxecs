require 'test/unit'
require 'ConfigServiceDriver.rb'
require 'ApiTestCase.rb'

class PermissionServiceTestApi < ApiTestCase
  
  def setup
    super
  
    # create the services that we will use
    @permission_service = service(PermissionService)
    @test_service = service(TestService)
    
    # reset the permission service so we start the test with no custom permissions
    reset = ResetServices.new();
    reset.permission = true;
    @test_service.resetServices(reset)

    # Save the number of built-in permissions
    builtInPermissions = @permission_service.findPermission(FindPermission.new()).permissions
    @builtInSize = builtInPermissions.length

  end
  
  def test_builtInPermissionsSize
    # Non-empty set of built-in permissions
    assert(@builtInSize>0, 'Not found built-in permissions')
  end

  def test_createPermission	    
    # Create a single custom permissions and query it by label
    expected = Permission.new('PermissionName', 'PermissionLabel', 'Description: permit to call', true)
    addPermission = AddPermission.new(expected)
    @permission_service.addPermission(addPermission)
    
    findPermission = FindPermission.new(PermissionSearch.new())
    findPermission.search.byLabel = expected.label
    permissions = @permission_service.findPermission(findPermission).permissions    
    assert_equal(1, permissions.length)
    assert_equal(expected.description, permissions[0].description)
  end
  
  def test_findBuiltInPermission
    expected = Permission.new('Voicemail', 'Voicemail', '', true, 'CALL', true)

    # Query built-in permission Voicemail by name
    findPermission = FindPermission.new(PermissionSearch.new())
    findPermission.search.byName = 'Voicemail'
    permissions = @permission_service.findPermission(findPermission).permissions
    assert_equal(1, permissions.length)
    assert_equal(expected.label, permissions[0].label)
    assert_equal(expected.name, permissions[0].name)
    assert_equal(expected.builtIn, permissions[0].builtIn)
    assert_equal(expected.type, permissions[0].type)

    # Query built-in permission Voicemail by label
    findPermission = FindPermission.new(PermissionSearch.new())
    findPermission.search.byLabel = 'Voicemail'
    permissions = @permission_service.findPermission(findPermission).permissions
    assert_equal(1, permissions.length)
    assert_equal(expected.label, permissions[0].label)
    assert_equal(expected.name, permissions[0].name)
    assert_equal(expected.builtIn, permissions[0].builtIn)
    assert_equal(expected.type, permissions[0].type)

  end

  def test_findCustomPermission

    perm1 = Permission.new('DummyName1', 'perm_11')
    @permission_service.addPermission(AddPermission.new(perm1))
    perm2 = Permission.new('DummyName2', '2_perm_22')
    @permission_service.addPermission(AddPermission.new(perm2))
    perm3 = Permission.new('DummyName3', 'perm_33', 'description')
    @permission_service.addPermission(AddPermission.new(perm3))
    perm4 = Permission.new('DummyName4', 'perm')
    @permission_service.addPermission(AddPermission.new(perm4))

    findPermission = FindPermission.new(PermissionSearch.new())
    findPermission.search.byLabel = 'perm_33'
    permissions = @permission_service.findPermission(findPermission).permissions
    assert_equal(1, permissions.length)
    assert_equal('CALL', permissions[0].type)
    assert_equal(false, permissions[0].builtIn)
    assert_equal(perm3.description, permissions[0].description)

  end

  def test_deletePermission

    initPermissions = @permission_service.findPermission(FindPermission.new()).permissions
    initSize = initPermissions.length

    @permission_service.addPermission(AddPermission.new(Permission.new('permissionName', 'permissionLabel')))
    
    search = PermissionSearch.new();
    search.byLabel = 'permissionLabel'
    permissions = @permission_service.findPermission(FindPermission.new(search)).permissions
    assert_equal(1, permissions.length)
    
    deletePermission = ManagePermission.new(search);
    deletePermission.deletePermission = true
    @permission_service.managePermission(deletePermission)
    
    permissions = @permission_service.findPermission(FindPermission.new(search)).permissions
    assert_equal(0, permissions.length)

    resultPermissions = @permission_service.findPermission(FindPermission.new()).permissions
    resultSize = resultPermissions.length
    assert_equal(resultSize, initSize)
    assert(resultSize>=@builtInSize)

  end

  def test_modifyPermission

    perm1 = Permission.new('Name1', 'mission')
    @permission_service.addPermission(AddPermission.new(perm1))
    perm2 = Permission.new('Name2', 'rmission')
    @permission_service.addPermission(AddPermission.new(perm2))
    perm3 = Permission.new('Name3', 'ermission', 'description', true)
    @permission_service.addPermission(AddPermission.new(perm3))
    perm4 = Permission.new('Name4', 'permission')
    @permission_service.addPermission(AddPermission.new(perm4))

    findPermission = FindPermission.new(PermissionSearch.new())
    findPermission.search.byLabel = 'ermission'
    permissions = @permission_service.findPermission(findPermission).permissions
    assert_equal(1, permissions.length)
    assert_equal('CALL', permissions[0].type)
    assert_equal(false, permissions[0].builtIn)
    assert_equal(true, permissions[0].defaultValue)
    assert_equal(perm3.description, permissions[0].description)

    # Change description and default value of a single custom permission
    managePermission = ManagePermission.new(PermissionSearch.new());
    managePermission.search.byLabel = 'ermission'
    managePermission.edit = [
    Property.new('description', 'New exciting description'), 
    Property.new('defaultValue', false) 
    ]
    @permission_service.managePermission(managePermission)

    findPermission = FindPermission.new(PermissionSearch.new())
    findPermission.search.byLabel = 'ermission'
    permissions = @permission_service.findPermission(findPermission).permissions
    assert_equal(1, permissions.length)
    assert_equal('CALL', permissions[0].type)
    assert_equal(false, permissions[0].builtIn)
    assert_equal(false, permissions[0].defaultValue)
    assert_equal('New exciting description', permissions[0].description)

    managePermission = ManagePermission.new();
    managePermission.edit = [
    Property.new('description', 'New common description'), 
    Property.new('defaultValue', true) 
    ]
    @permission_service.managePermission(managePermission)

    permissions = @permission_service.findPermission(FindPermission.new()).permissions

    for perm in permissions
      if false == perm.builtIn()
        assert_equal(true, perm.defaultValue)
        assert_equal('New common description', perm.description)
      end
    end
  end

  def test_deletePermissions
    managePermission = ManagePermission.new();
    managePermission.deletePermission = true
    @permission_service.managePermission(managePermission)

    permissions = @permission_service.findPermission(FindPermission.new()).permissions

    for perm in permissions
      assert_equal(true, perm.builtIn())
    end

    assert_equal(permissions.length, @builtInSize)
  end

end