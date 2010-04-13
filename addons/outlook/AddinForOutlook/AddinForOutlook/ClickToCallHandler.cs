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
using System.Collections;

using System.Windows.Forms;
using Microsoft.Office.Core;
using Microsoft.Office.Interop.Outlook;

namespace AddinForOutlook
{
    public class ClickToCalllHandler
    {
        private Object _item = null;
        public ClickToCalllHandler(Object item)
        {
            this._item = item;
        }


        public void start()
        {
            try
            {
                if (_item is AppointmentItem)
                {
                    AppointmentItem theItem = _item as AppointmentItem;
                    String conferenceNumber = Utility.getConferenceNumber(theItem.Location);

                    InputConfirmWindow ccwnd = null;
                    if (conferenceNumber == null)
                    {
                        ccwnd = new InputConfirmWindow(Error.ERROR_MSG_NO_CONFERENCE_NUMBER, String.Empty);
                    }

                    else if (Utility.checkInvalidLetters(conferenceNumber))
                    {
                        ccwnd = new InputConfirmWindow(Error.ERROR_MSG_INVALID_CONFERENCE_NUMBER, conferenceNumber);
                    }

                    else
                    {
                        ccwnd = new InputConfirmWindow(Error.CONFIRM_MSG_CONFERENCE_NUMBER, conferenceNumber);
                    }

                    if (DialogResult.OK != ccwnd.ShowDialog()) return;

                    conferenceNumber = ccwnd.DestinationNumber;
                    RestInterface.restCall(conferenceNumber, Utility.getSCSAccountPassWord());
                }
                else if (_item is MailItem)
                {
                    MailItem theItem = _item as MailItem;

                    Hashtable phoneNumberList = null;
                    phoneNumberList = InspectorWrapper.resolveContactNumberFromLocal(theItem.SenderEmailAddress, theItem.SenderName);
                    if (phoneNumberList == null)
                    {
                        phoneNumberList = InspectorWrapper.resolveContactNumberFromGAL(theItem.SenderName);
                    }

                    String calledNumber = String.Empty;
                    if (phoneNumberList == null)
                    {
                        InputConfirmWindow ccwnd = new InputConfirmWindow(Error.ERROR_MSG_NO_SENDER_NUMBER, String.Empty);
                        if (DialogResult.OK != ccwnd.ShowDialog()) return;
                        calledNumber = ccwnd.DestinationNumber;
                    }
                    else
                    {
                        ComboConfirmWindow ccwnd = new ComboConfirmWindow("Sure to call the sender at this number? (or pick/type a new number)", phoneNumberList);
                        if (DialogResult.Yes != ccwnd.ShowDialog()) return;
                        calledNumber = ccwnd.DestinationNumber;
                    }

                    RestInterface.restCall(calledNumber, Utility.getSCSAccountPassWord());
                }
                else if (_item is ContactItem)
                {
                    ContactItem theItem = _item as ContactItem;

                    Hashtable phonelist = InspectorWrapper.getPhoneList(theItem);

                    String calledNumber = string.Empty;
                    if (phonelist == null)
                    {
                        InputConfirmWindow ccwnd = new InputConfirmWindow(Error.ERROR_MSG_NO_CONTACT_NUMBER, String.Empty);
                        if (DialogResult.OK != ccwnd.ShowDialog()) return;
                        calledNumber = ccwnd.DestinationNumber;
                    }

                    else
                    {
                        ComboConfirmWindow ccwnd = new ComboConfirmWindow(Error.CONFIRM_MSG_CONTACT_NUMBER, phonelist);
                        if (DialogResult.Yes != ccwnd.ShowDialog()) return;
                        calledNumber = ccwnd.DestinationNumber;
                    }

                    RestInterface.restCall(calledNumber, Utility.getSCSAccountPassWord());
                }
            }

            catch (System.Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

        }
    }
}
