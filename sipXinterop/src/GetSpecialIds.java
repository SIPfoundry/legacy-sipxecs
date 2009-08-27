/*
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

import org.sipfoundry.sipxconfig.common.SpecialUser;

public class GetSpecialIds {
	
   public static void main(String [] args) { 
   		for (SpecialUser.SpecialUserType su : SpecialUser.SpecialUserType.values())
           		System.out.print(su.getUserName() + "\\/" + su + "SHELL_FUN_DELIMITER" );
   }

}
