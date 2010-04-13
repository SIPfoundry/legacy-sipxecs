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
using System.Windows.Forms;
using Extensibility;
using System.Runtime.InteropServices;
using Microsoft.Office.Core;
using Microsoft.Office.Interop.Outlook;
using System.Net;
using System.IO;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Security.Cryptography.X509Certificates;
using System.Globalization;
using System.Net.Security;
using System.Security;


namespace AddinForOutlook
{
    public class InspectorWrapper
    {
        public static Microsoft.Office.Interop.Outlook.Application TheOutlookApplication;
        public static ItemEvents_10_SendEventHandler sendHander = new ItemEvents_10_SendEventHandler(AppointmentItem_Send);

        private Inspector inspector;
        private InspectorEvents_ActivateEventHandler TheActivateHandler;

        public InspectorWrapper(Inspector inspector)
        {
            this.inspector = inspector;
            TheActivateHandler = new InspectorEvents_ActivateEventHandler(activateHandler);
            ((Microsoft.Office.Interop.Outlook.InspectorEvents_Event)this.inspector).Activate += TheActivateHandler;

            if (this.inspector.CurrentItem is AppointmentItem)
            {
                AppointmentItem theItem = this.inspector.CurrentItem as AppointmentItem;
                ((Microsoft.Office.Interop.Outlook.ItemEvents_10_Event)theItem).Send += sendHander;
            }

            InitializeInspector();
        }

        /// <summary>
        /// 
        /// </summary>
        public void activateHandler()
        {
            if (inspector.CurrentItem is AppointmentItem)
            {
                
                AppointmentItem theItem = inspector.CurrentItem as AppointmentItem;
                if (((theItem.Subject == null) || (theItem.Subject.Length == 0)) &&
                     ((theItem.Location == null) || (theItem.Location.Length == 0)))
                {
                    if (Utility.isSCSDefaultLocation())
                    {
                        theItem.Location = Utility.getDefaultLocationFromReg();
                    }
                }
            }

        }

        /// <summary>
        /// 
        /// </summary>
        public void InitializeInspector()
        {
            activateHandler();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="cmdBarbutton"></param>
        /// <param name="cancel"></param>
        public static void scsmeetingButton_Click(CommandBarButton notused, ref bool cancel)
        {
            AppointmentItem theItem = null;
            if (TheOutlookApplication.ActiveInspector().CurrentItem is AppointmentItem)
            {
                theItem = TheOutlookApplication.ActiveInspector().CurrentItem as AppointmentItem;
            }
            else
            {
                Logger.WriteEntry(LogLevel.Error, "The button is clicked for a non-appointment item!");
                MessageBox.Show("This button is only applicable to an AppointmentItem!");
                return;
            }

            Microsoft.Win32.RegistryKey key;
            key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);

            String conferenceNumber = (String)key.GetValue(Constants.REG_ATTR_NAME_CONFERENCE_NUMBER);
            String conferenceAccessCode = (String)key.GetValue(Constants.REG_ATTR_NAME_CONFERENCE_ACCESS_CODE);
            key.Close();

            String scsLocation = Utility.buildLocationString(conferenceNumber, conferenceAccessCode);

            theItem.Location = scsLocation;

        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="emailaddress"></param>
        /// <returns></returns>
        public static ContactItem getMatchingContactByEmail(MAPIFolder folder, String emailaddress)
        {
            foreach (Object item in folder.Items)
            {
                if (item is ContactItem)
                {
                    ContactItem theItem = item as ContactItem;

                    if ((theItem.Email1Address != String.Empty && theItem.Email1Address == emailaddress) ||
                        (theItem.Email2Address != String.Empty && theItem.Email1Address == emailaddress) ||
                        (theItem.Email3Address != String.Empty && theItem.Email1Address == emailaddress))
                    {
                        return theItem;
                    }
                }
                else // The folder can only include certain type of items. 
                {
                    return null;
                }
            }

            return null;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="contactName"></param>
        /// <returns></returns>
        public static Hashtable resolveContactNumberFromLocal(String emailaddress, String contactName)
        {
            MAPIFolder cntctFolder =  (MAPIFolder)TheOutlookApplication.Session.GetDefaultFolder(OlDefaultFolders.olFolderContacts);
            ContactItem theItem = null;

            theItem = getMatchingContactByEmail(cntctFolder, emailaddress);
            if (theItem != null) goto found;

            foreach (MAPIFolder subFolder in cntctFolder.Folders)
            {
                if (subFolder.DefaultItemType == OlItemType.olContactItem) // could be either ContactItem or DistListItem
                {
                    theItem = getMatchingContactByEmail(subFolder, emailaddress);
                    if (theItem != null) goto found;
                }
            }
            
            if (theItem == null) return null;
             
            
            found:
                            
            return getPhoneList(theItem);
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="theItem"></param>
        /// <returns></returns>
        public static Hashtable getPhoneList(ContactItem theItem)
        {
            Hashtable phoneList = new Hashtable();

            try
            {
                if ((theItem.BusinessTelephoneNumber != null) && (theItem.BusinessTelephoneNumber != String.Empty))
                {
                    phoneList.Add(Constants.KEY_BUSINESS_PHONE, normalizeNumber(theItem.BusinessTelephoneNumber));
                }

                if ((theItem.Business2TelephoneNumber != null) && (theItem.Business2TelephoneNumber != String.Empty))
                {
                    phoneList.Add(Constants.KEY_BUSINESS_PHONE_2, normalizeNumber(theItem.Business2TelephoneNumber));
                }

                if ((theItem.CompanyMainTelephoneNumber != null) && (theItem.CompanyMainTelephoneNumber != String.Empty))
                {
                    phoneList.Add(Constants.KEY_COMPANY_PHONE, normalizeNumber(theItem.CompanyMainTelephoneNumber));
                }

                if ((theItem.HomeTelephoneNumber != null) && (theItem.HomeTelephoneNumber != String.Empty))
                {
                    phoneList.Add(Constants.KEY_HOME_PHONE, normalizeNumber(theItem.HomeTelephoneNumber));
                }

                if ((theItem.MobileTelephoneNumber != null) && (theItem.MobileTelephoneNumber != String.Empty))
                {
                    phoneList.Add(Constants.KEY_MOBILE_PHONE, normalizeNumber(theItem.MobileTelephoneNumber));
                }

                if ((theItem.CarTelephoneNumber != null) && (theItem.CarTelephoneNumber != String.Empty))
                {
                    phoneList.Add(Constants.KEY_CAR_PHONE, normalizeNumber(theItem.CarTelephoneNumber));
                }

                if (phoneList.Count == 0)
                {
                    return null;
                }
                else
                {
                    return phoneList;
                }
            }
            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, ex.GetType() + " : " + ex.Message);
                return null;
            }

        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="contactNumber"></param>
        /// <returns></returns>
        /// 

        public static Hashtable resolveContactNumberFromGAL(String contactName)
        {
            try
            {
                Hashtable phonelist = new Hashtable();
                AddressEntry entry = TheOutlookApplication.Session.GetGlobalAddressList().AddressEntries[contactName];

                try
                {
                    String btn = (String)entry.PropertyAccessor.GetProperty(Constants.PROPTAG_BUSINESS_PHONE);
                    phonelist.Add(Constants.KEY_BUSINESS_PHONE, normalizeNumber(btn));
                }

                catch (System.Exception ex)
                {
                    Logger.WriteEntry(LogLevel.Information, ex.GetType() + " : " + ex.Message);
                }


                try
                {
                    String mobile = (String)entry.PropertyAccessor.GetProperty(Constants.PROPTAG_MOBILE_PHONE);
                    phonelist.Add(Constants.KEY_MOBILE_PHONE, normalizeNumber(mobile));
                }

                catch (System.Exception ex)
                {
                    Logger.WriteEntry(LogLevel.Information, ex.GetType() + " : " + ex.Message);
                }

                try
                {
                    String home = (String)entry.PropertyAccessor.GetProperty(Constants.PROPTAG_HOME_PHONE);
                    phonelist.Add(Constants.KEY_HOME_PHONE, normalizeNumber(home));
                }
                catch (System.Exception ex)
                {
                    Logger.WriteEntry(LogLevel.Information, ex.GetType() + " : " + ex.Message);
                }


                int refsLeft = 0;
                do
                {
                    refsLeft = Marshal.ReleaseComObject(entry);
                }
                while (refsLeft > 0);


                if (phonelist.Count == 0)
                {
                    return null;
                }
                else
                {
                    return phonelist;
                }

            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, ex.GetType() + " : " + ex.Message);
                return null;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="cancel"></param>
        public static void AppointmentItem_Send(ref bool cancel)
        {
            if (TheOutlookApplication.ActiveInspector().CurrentItem is _AppointmentItem)
            {
                AppointmentItem theItem = TheOutlookApplication.ActiveInspector().CurrentItem as AppointmentItem;
                if (String.Compare(theItem.Location, Constants.SERVER_FLAG.Trim()) == 0)
                {
                    theItem.Location = Utility.getDefaultLocationFromReg();
                }
            }
        }




        /// <summary>
        /// 
        /// </summary>
        /// <param name="phonenumber"></param>
        /// <returns></returns>
        public static String normalizeNumber(String phonenumber)
        {
            Microsoft.Win32.RegistryKey key;

            key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);
            String prefixtoremove = (String)key.GetValue(Constants.REG_ATTR_NAME_PREFIX_TO_REMOVE);
            String prefixtoadd = (String)key.GetValue(Constants.REG_ATTR_NAME_PREFIX_TO_ADD);

            key.Close();            

            phonenumber = Utility.removeNonNumbers(phonenumber);

            if (phonenumber.StartsWith(prefixtoremove))
            {
                phonenumber =  phonenumber.Substring(prefixtoremove.Length);
            }
            
            if (prefixtoadd != null)
            {
                phonenumber = prefixtoadd + phonenumber;
            }

            return phonenumber;
        }


    }
}
