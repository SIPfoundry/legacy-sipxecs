//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

using System;
using System.Collections.Generic;
using System.Text;

namespace AddinForOutlook
{
    public class Constants
    {
        public const string DEFAULT_SERVER_BRANDING = "sipX";
        public const int MAXIMU_LENGTH_SERVER_BRANDING = 16;

        //
        public static string SERVER_FLAG = Utility.getServerBranding();
        

        //For Logging.
        public static string PRODUCT_NAME = SERVER_FLAG + "AddinForOutlook";

        //Registry related
        //Registry key
        public const string REG_KEY_NAME_ADDIN_SETTING = "Software\\Microsoft\\Office\\Outlook\\Addins\\AddinForOutlook.Connect\\custom";

        //Registery property name
        public const string REG_ATTR_NAME_CONFERENCE_NUMBER = "conference_number";
        public const string REG_ATTR_NAME_CONFERENCE_ACCESS_CODE = "conference_access_code";
        public const string REG_ATTR_NAME_USERNAME = "username";
        public const string REG_ATTR_NAME_CTI_SERVER_NAME = "cti_server_name";
        public const string REG_ATTR_NAME_PORT = "port";
        public const string REG_ATTR_NAME_ISDEFAULTLOCATION = "isDefaultLocation";
        public const string REG_ATTR_NAME_ISPINREMEMBERED = "isPINRemembered";
        public const string REG_ATTR_NAME_USER_PIN = "user_pin";
        public const string REG_ATTR_NAME_PREFIX_TO_ADD = "prefixtoadd";
        public const string REG_ATTR_NAME_PREFIX_TO_REMOVE = "prefixtoremove";
        public const string REG_ATTR_NAME_LOG_LEVEL = "log_level";
        public const string REG_ATTR_NAME_SERVER_BRANDING = "server_branding";
        
        // Default value
        public const string DEFAULT_CTI_SERVER_NAME = "localhost";
        public const string DEFAULT_CTI_SERVICE_PORT = "8443";
        public const string DEFAULT_USER_NAME = "anonymous";
        public const string DEFAULT_PASSWORD = "unknown";
        public const string DEFAULT_CONFERENCE_NUMBER =  "unknown";
        public const string DEFAULT_PASSCODE = "unknown";
        public const int DEFAULT_IS_DEFAULT_LOCATION = 1;
        public const int DEFAULT_ISPINREMEMBERED = 1;

        //REST API synopsis
        public const string REST_VIRTUAL_PATH = "/sipxconfig/rest/call/";
        public const string REST_METHOD = "PUT";

        //Meeting location setting

        public const double MAXIUM_MEETING_DELAY = -60; //60 min
        public const double MAXIUM_MEETING_ADVANCE = 30; //30 min

        //
        // VERY IMPORTANT: DO NOT CHANGE CONFERENCE NUMBER FLAG, 
        // IT MAY CAUSE INCOMPATIBILITY BETWEEN DIFFERENT BUILDS
        // THE *CLICK TO CONFERENCE* CALL MAY NOT WORK
        //
        public const string CONFERENCE_NUMBER_FLAG = "@CN";
  
        public static string CONFERENCE_LOCATION_FLAG = SERVER_FLAG + CONFERENCE_NUMBER_FLAG;
        public const char FIELD_SEPERATOR = ' ';
        public const string CONFERENCE_ACCESS_CODE_FLAG = "ConferenceAccessCode";

        //GUI Related
        public const string CAPTION_START_BUTTON = "Start";
        public const string CAPTION_JOIN_BUTTON = "Join";
        public const string DESC_START_JOIN_BUTTON = "Call to conference";

        public static string CAPTION_CALL_CONTEXT_BUTTON = SERVER_FLAG + " Call";
        public const string DESC_CALL_BUTTON = "Call";

        public const string TAG_SCS_CONFIG_BUTTON_IN_TOOLS = "xxaForConfigurationOLaDDiNxxx";
        public static string CAPTION_SCS_CONFIG_BUTTON_IN_TOOLS = SERVER_FLAG + " Configuration";
        public const string DESC_SCS_CONFIG_BUTTON_IN_TOOLS = " configuration";


        public const string TAG_SCS_ACCOUNT_SETTING_BUTTON = "xxaForSettingOLaDDiNxxx";
        public const string CAPTION_SCS_ACCOUNT_SETTING_BUTTON = "Setting";
        public const string DESC_SCS_ACCOUNT_SETTING_BUTTON = "Setting ";

        public const string TAG_SCS_ACCOUNT_PASSCODE_BUTTON = "xxaForPasswordOLaDDiNxxx";
        public const string CAPTION_SCS_ACCOUNT_PASSCODE_BUTTON = "Set Account PIN";
        public const string DESC_SCS_ACCOUNT_PASSCODE_BUTTON = "Account PIN";

        public const string TAG_SCS_CALL_RULES_BUTTON = "xxaForCallRulesOLaDDiNxxx";
        public const string CAPTION_SCS_CALL_RULES_BUTTON = "Call Rules";
        public const string DESC_SCS_CALL_RULES_BUTTON = "Call Rules";

        public const string TAG_START_APPOINTMENTITEM_CONTEXT_MENU = "START_CONTEXT_MENU_TAG_FOR_APPOINTMENT";
        public const string TAG_CALL_CONTACT_CONTEXT_MENU = "CALL_CONTEXT_MENU_TAG_FOR_CONTACT";
        public const string TAG_CALL_MAIL_CONTEXT_MENU = "CALL_CONTEXT_MENU_TAG_FOR_MAIL";

        public const string PREFIX_DESC = "<";
        public const string SUBFIX_DESC = ">";

        //Property tag
        public const string PROPTAG_HOME_PHONE = "http://schemas.microsoft.com/mapi/proptag/0x3A09001E";
        public const string PROPTAG_MOBILE_PHONE = "http://schemas.microsoft.com/mapi/proptag/0x3A1C001E";
        public const string PROPTAG_BUSINESS_PHONE = "http://schemas.microsoft.com/mapi/proptag/0x3A08001E";

        public const string KEY_BUSINESS_PHONE = "Business";
        public const string KEY_BUSINESS_PHONE_2 = "Business2";
        public const string KEY_MOBILE_PHONE = "Mobile";
        public const string KEY_HOME_PHONE = "Home";
        public const string KEY_COMPANY_PHONE = "Company";
        public const string KEY_CAR_PHONE = "Car";
    }
}
