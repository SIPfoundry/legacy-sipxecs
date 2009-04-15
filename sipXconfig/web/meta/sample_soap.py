# using sipXconfig SOAP from Python - example
# submitted by Joey Korkames<joey@limelightnetworks.com>

    sipx_host = 'sipxserver.example.com'

    from SOAPpy import WSDL
    from SOAPpy import URLopener
    from SOAPpy import Types

    # need to marshall into SOAP types
    SOAP_FALSE = Types.booleanType(0)
    SOAP_TRUE = Types.booleanType(1)

    #I've manually disabled HTTP auth in my sipxconfig installation (to debug other issues), so the auth here may not actually work in a stock sipx environment....
    sipx_auth = URLopener.URLopener(username='sipxsoap',passwd='scrubscrub')

    sipx_namespace = 'urn:ConfigService'

    sipx_user_wsdl = 'http://' + sipx_host + '/sipxconfig/services/UserService?wsdl'
    sipx_user = WSDL.Proxy(sipx_auth.open(sipx_user_wsdl), namespace=sipx_namespace)

    sipx_phone_wsdl = 'http://' + sipx_host + '/sipxconfig/services/PhoneService?wsdl'
    sipx_phone = WSDL.Proxy(sipx_auth.open(sipx_phone_wsdl), namespace=sipx_namespace)

    #prints SOAP traffic to STDOUT
    #sipx_user.soapproxy.config.dumpSOAPOut = 1
    #sipx_user.soapproxy.config.dumpSOAPIn = 1
    #sipx_phone.soapproxy.config.dumpSOAPOut = 1
    #sipx_phone.soapproxy.config.dumpSOAPIn = 1


    for user in users:

      #user is a python dictionary of a person to add to a sipx system. Currently, users here are labeled after their phone's MACs (which stinks)...

        userdata = {'userName': user["sipmac"] + '-1', 'pintoken': '9999', 'lastName': user["lastname"], 'firstName': user["firstname"], 'sipPassword': 'seekrutz', 'groups': 'primary-group'}
        try:
          sipx_user.addUser(user=userdata,pin='9999')
          print>>log, 'Added SipX user'
        except:
          print>>log, 'Failed to add SipX user - maybe user ' + user["sipmac"] + '-1'+ ' already exists?'

        usersearch = {'byUserName': user["sipmac"] + '-1'}
        try:
          sipx_user.manageUser(search=usersearch,addGroup='secondary-group')
          print>>log, 'Added SipX user to secondary-group'
        except:
          pass

        linedata = {'userId': user["sipmac"] + '-1', 'uri': '<sip:' + user["sipmac"] + '-1' + '@' + 'example.com>'}

        #voictel is a 10-digit phone number
        phonedata = {'serialNumber': user["sipmac"], 'modelId': 'ciscoIp7960', 'description': user["firstname"] + ' ' + user["lastname"] + ' ' + '(' + user['voicetel'][:3] + ') ' + user['voicetel'][3:6] + '-' + user['voicetel'][-4:], 'groups':'primary-group' }

        try:
         sipx_phone.addPhone(phone=phonedata)
         print 'Added SipX phone'
        except:
          print 'Failed to add SipX phone'

        phonesearch = {'bySerialNumber': user["sipmac"]}
        try:
         sipx_phone.managePhone(search=phonesearch, addGroup='secondary-group')
         print 'Added SipX phone to secondary-group'
        except:
          print 'Failed to add SipX phone to secondary-group'

        try:
         sipx_phone.managePhone(search=phonesearch, addLine=linedata)
         print 'Edited SipX phone: line added'
        except:
          print 'Failed to add SipX line'

        try:
         sipx_phone.managePhone(search=phonesearch, generateProfiles=SOAP_TRUE)
         print 'Pushed SipX phone config to TFTP store'
        except:
          print 'Failed to push SipX phone config to TFTP store'

