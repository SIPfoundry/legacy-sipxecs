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
    public class Error
    {
        public const String ERROR_MSG_NO_CONFERENCE_NUMBER = "No conference number found! Please type one:";
        public const String ERROR_MSG_INVALID_CONFERENCE_NUMBER = "Invalid letter found! please define a valid conference number:";
        public const String CONFIRM_MSG_CONFERENCE_NUMBER = "Sure to dial in at this number? or type a new number:";

        public const String ERROR_MSG_NO_CONTACT_NUMBER = "No phone number found for this contact! please type one:";
        public const String ERROR_MSG_INVALID_CONTACT_NUMBER = "Invalid letter found! please define a valid phone number:";
        public const String CONFIRM_MSG_CONTACT_NUMBER = "Sure to call the contact at this number? Or pick/type a new number:";

        public const String ERROR_MSG_NO_SENDER_NUMBER = "No phone number found for the sender! please type one:";
        public const String ERROR_MSG_INVALID_SENDER_NUMBER = "Invalid letter found! please define a valid phone number:";
        public const String CONFIRM_MSG_SENDER_NUMBER = "Sure to call the sender at this number? Or pick/type a new number:";        
    }
}
