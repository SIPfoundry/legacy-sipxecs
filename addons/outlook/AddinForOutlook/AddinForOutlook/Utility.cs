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
using System.Net;
using System.Windows.Forms;
using Microsoft.Office.Core;
using Microsoft.Office.Interop.Outlook;

namespace AddinForOutlook
{
    public static class Utility
    {
        /// <summary>
        /// 
        /// </summary>
        /// <param name="cbar"> commandBar on which the button is added.</param>
        /// <param name="tag"> button tag</param>
        /// <param name="desc">button description</param>
        /// <param name="caption">button caption</param>
        /// <param name="before"></param>
        /// <param name="temp"></param>
        /// <param name="handler"></param>
        /// <param name="parameter"></param>
        /// <returns></returns>
        public static CommandBarButton addButton(CommandBar cbar, String tag, String desc, 
            String caption, Object before, bool temp, 
            _CommandBarButtonEvents_ClickEventHandler handler, String parameter)
        {

            CommandBarButton foundButton = (CommandBarButton)cbar.FindControl(
                MsoControlType.msoControlButton,
                Type.Missing, tag, false, true);

            if (foundButton != null)
            {
                foundButton.Caption = caption;
                setClickHandler(foundButton, handler);
                foundButton.Visible = true;
                foundButton.Parameter = parameter;
                return foundButton;
            }

            CommandBarButton button = (CommandBarButton)cbar.Controls.Add(
                MsoControlType.msoControlButton,
                Type.Missing, Type.Missing, before, temp);

            setClickHandler(button, handler);

            button.Tag = tag;
            button.DescriptionText = desc;
            button.BeginGroup = false;
            button.Caption = caption;
            button.Style = MsoButtonStyle.msoButtonIconAndCaption;
            button.Visible = true;
            button.Parameter = parameter;

            return button;
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="theBar"></param>
        /// <param name="tag"></param>
        public static void RemoveButton(CommandBar theBar, String tag)
        {
            try
            {
                CommandBarButton foundButton = (CommandBarButton)
                    theBar.FindControl(MsoControlType.msoControlButton,
                    Type.Missing, tag, true, true);
                if (foundButton != null)
                {
                    foundButton.Delete(false);
                    foundButton = null;
                }
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Warning, ex.GetType() + ":" + ex.Message);
            }

        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="button"></param>
        /// <param name="handler"></param>
        public static void setClickHandler(CommandBarButton button, _CommandBarButtonEvents_ClickEventHandler handler)
        {

            // Make sure it only fires once.
            try
            {
                do
                {
                    button.Click -= handler;
                } while (true);
            }
            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Information, "all click handlers have been successfuly detached from button " + button.Caption + " " + ex.Message);
            }

            finally
            {
                button.Click += handler;
            }

        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="loc"></param>
        /// <returns></returns>
        public static String getConferenceNumber(String loc)
        {            
            int startIndex = loc.IndexOf(Constants.CONFERENCE_NUMBER_FLAG);
            if (startIndex == -1)
            {
                return null;
            }

            if (loc.Length <= startIndex + Constants.CONFERENCE_NUMBER_FLAG.Length + 1)
            {
                return null;
            }


            String sub = loc.Substring(startIndex + Constants.CONFERENCE_NUMBER_FLAG.Length + 1);
            int endIndex = sub.IndexOf(" "); // Space as sperator.
            if (endIndex == -1)
            {
                return sub;
            }

            return sub.Substring(0, endIndex);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="scs_fqdn"></param>
        /// <returns></returns>
        public static String getCtiServerIPAddress(String hostnameOrIPAddress)
        {
            try
            {
                IPAddress[] addresslist = Dns.GetHostAddresses(hostnameOrIPAddress);
                foreach (IPAddress theaddress in addresslist)
                {
                    return theaddress.ToString(); // We pick the first one, don't ask why.
                }

                return null;
            }
            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Failed to get Cti server IP address! Exception : " + ex.Message + ex.StackTrace);
                return null;
            }

        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public static bool isSCSDefaultLocation()
        {
            try
            {
                Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);

                int val = (int)key.GetValue(Constants.REG_ATTR_NAME_ISDEFAULTLOCATION);
                bool isDefaultLocation = (val == 0 ? false : true);

                key.Close();

                return isDefaultLocation;
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Failed to check default location! Exception : " + ex.Message + ex.StackTrace);
                return false;
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public static string getDefaultLocationFromReg()
        {

            try
            {
                Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);

                String conferenceNumber = (String)key.GetValue(Constants.REG_ATTR_NAME_CONFERENCE_NUMBER);
                String conferenceAccessCode = (String)key.GetValue(Constants.REG_ATTR_NAME_CONFERENCE_ACCESS_CODE);
                key.Close();

                return buildLocationString(conferenceNumber, conferenceAccessCode);
            }
            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Failed to get default location! Exception : " + ex.Message + ex.StackTrace);
                return "";
            }

        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="conferenceNumber"></param>
        /// <param name="conferenceAccessCode"></param>
        /// <returns></returns>
        public static String buildLocationString(String conferenceNumber, String conferenceAccessCode)
        {
            StringBuilder sb = new StringBuilder(Constants.CONFERENCE_LOCATION_FLAG);
            sb.Append(Constants.FIELD_SEPERATOR);
            sb.Append(conferenceNumber);
            sb.Append(Constants.FIELD_SEPERATOR);
            sb.Append(Constants.CONFERENCE_ACCESS_CODE_FLAG);
            sb.Append(Constants.FIELD_SEPERATOR);
            sb.Append(conferenceAccessCode);
            return sb.ToString();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="wnd"></param>
        /// <returns></returns>
        public static String getSCSAccountPassWord()
        {
            if (PassCodeWindow.SCSPassword != String.Empty)
            {
                return PassCodeWindow.SCSPassword;
            }

            try
            {
                PassCodeWindow wnd = new PassCodeWindow();
                wnd.ShowDialog();

                return PassCodeWindow.SCSPassword;
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Exception : " + ex.Message + ex.StackTrace);
                return "unknown";
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="str"></param>
        /// <returns></returns>
        public static String removeNonNumbers(String str)
        {
            char[] allchars = str.ToCharArray();
            StringBuilder sb = new StringBuilder(String.Empty);

            foreach (char ch in allchars)
            {
                if (ch >= '0' && ch <= '9')
                {
                    sb.Append(ch);
                }
            }

            return sb.ToString();
        }


        /// <summary>
        /// 
        /// </summary>
        /// <param name="str"></param>
        /// <returns></returns>
        public static bool checkInvalidLetters(String str)
        {
            char[] allchars = str.ToCharArray();
            StringBuilder sb = new StringBuilder(String.Empty);

            foreach (char ch in allchars)
            {
                if ((ch < '0' || ch > '9') && (ch < 'A' || ch > 'Z') && (ch < 'a' || ch > 'z'))
                {
                    return true;
                }
            }

            return false;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="str"></param>
        /// <returns></returns>
        public static bool checkNonNumbers(String str)
        {
            char[] allchars = str.ToCharArray();
            StringBuilder sb = new StringBuilder(String.Empty);

            foreach (char ch in allchars)
            {
                if ((ch < '0' || ch > '9'))
                {
                    return true;
                }
            }

            return false;
        }


        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public static string getServerBranding()
        {
            Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);
            string serverBranding = (String)key.GetValue(Constants.REG_ATTR_NAME_SERVER_BRANDING, Constants.DEFAULT_SERVER_BRANDING);
            key.Close();

            if (checkInvalidLetters(serverBranding) || serverBranding.Length > Constants.MAXIMU_LENGTH_SERVER_BRANDING)
            {
                return Constants.DEFAULT_SERVER_BRANDING;
            }
            return serverBranding;
        }
    }
}
