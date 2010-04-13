//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
namespace AddinForOutlook
{
	using System;
	using Extensibility;
	using System.Runtime.InteropServices;

    using System.Windows.Forms;
    using Microsoft.Office.Core;
    using Microsoft.Office.Interop.Outlook;
    using System.Net;
    using System.IO;
    using System.Drawing;
    using System.Collections;
    using System.ComponentModel;
    using System.Data;
    using System.Diagnostics;
    using System.Text;
    using System.Security.Cryptography.X509Certificates;
    using System.Globalization;
    using System.Net.Security;
    using System.Security;
    using System.Xml;

    
	#region Read me for Add-in installation and setup information.
	// When run, the Add-in wizard prepared the registry for the Add-in.
	// At a later time, if the Add-in becomes unavailable for reasons such as:
	//   1) You moved this project to a computer other than which is was originally created on.
	//   2) You chose 'Yes' when presented with a message asking if you wish to remove the Add-in.
	//   3) Registry corruption.
	// you will need to re-register the Add-in by building the SCSAddin4OutlookSetup project, 
	// right click the project in the Solution Explorer, then choose install.
	#endregion
	
	/// <summary>
	///   The object for implementing an Add-in.
	/// </summary>
	/// <seealso class='IDTExtensibility2' />
    [GuidAttribute("127F08A7-CF42-4C69-903A-8534A3772E23"), ProgId("AddinForOutlook.Connect")]
    public class Connect : Object, Extensibility.IDTExtensibility2, IRibbonExtensibility
	{
		/// <summary>
		///		Implements the constructor for the Add-in object.
		///		Place your initialization code within this method.
		/// </summary>
		public Connect()
		{
            _scs_button_handler = new _CommandBarButtonEvents_ClickEventHandler(scsButton_Click);
            _call_contact_button_handler = new _CommandBarButtonEvents_ClickEventHandler(ItemContext_Click);
            _call_sender_button_handler = new _CommandBarButtonEvents_ClickEventHandler(ItemContext_Click);
            _start_meeting_button_handler = new _CommandBarButtonEvents_ClickEventHandler(ItemContext_Click);
		}

		/// <summary>
		///      Implements the OnConnection method of the IDTExtensibility2 interface.
		///      Receives notification that the Add-in is being loaded.
		/// </summary>
		/// <param term='application'>
		///      Root object of the host application.
		/// </param>
		/// <param term='connectMode'>
		///      Describes how the Add-in is being loaded.
		/// </param>
		/// <param term='addInInst'>
		///      Object representing this Add-in.
		/// </param>
		/// <seealso class='IDTExtensibility2' />
		public void OnConnection(object application, Extensibility.ext_ConnectMode connectMode, object addInInst, ref System.Array custom)
		{
            _OutlookApplication = (Microsoft.Office.Interop.Outlook.Application)application;
            InspectorWrapper.TheOutlookApplication = _OutlookApplication;            
            _addInInstance = addInInst;

            if (connectMode != Extensibility.ext_ConnectMode.ext_cm_Startup)
            {
                OnStartupComplete(ref custom);
            }

		}

		/// <summary>
		///     Implements the OnDisconnection method of the IDTExtensibility2 interface.
		///     Receives notification that the Add-in is being unloaded.
		/// </summary>
		/// <param term='disconnectMode'>
		///      Describes how the Add-in is being unloaded.
		/// </param>
		/// <param term='custom'>
		///      Array of parameters that are host application specific.
		/// </param>
		/// <seealso class='IDTExtensibility2' />
		public void OnDisconnection(Extensibility.ext_DisconnectMode disconnectMode, ref System.Array custom)
		{
            if (disconnectMode != Extensibility.ext_DisconnectMode.ext_dm_HostShutdown)
            {
                OnBeginShutdown(ref custom);
            }

		}

		/// <summary>
		///      Implements the OnAddInsUpdate method of the IDTExtensibility2 interface.
		///      Receives notification that the collection of Add-ins has changed.
		/// </summary>
		/// <param term='custom'>
		///      Array of parameters that are host application specific.
		/// </param>
		/// <seealso class='IDTExtensibility2' />
		public void OnAddInsUpdate(ref System.Array custom)
		{
		}

		/// <summary>
		///      Implements the OnStartupComplete method of the IDTExtensibility2 interface.
		///      Receives notification that the host application has completed loading.
		/// </summary>
		/// <param term='custom'>
		///      Array of parameters that are host application specific.
		/// </param>
		/// <seealso class='IDTExtensibility2' />
        
		public void OnStartupComplete(ref System.Array custom)
		{
            try
            {
                InitializeLogging();
                AddSCSPopupButton();
                initializeEventsHandler();
                
                Logger.WriteEntry(LogLevel.Information, Constants.PRODUCT_NAME + " loadded successfully!");
            }

            catch (System.Exception e)
            {
                throw e;
            }

		}

        /// <summary>
        /// Set the logger to use File logging which is the only supported mechanism so far.
        /// </summary>
        void InitializeLogging()
        {
            Logger.Delegate = new LoggerDelegate(FileLogging.WriteEntry);

#if DEBUG
            MessageBox.Show(FileLogging.GenerateDefaultLogFileName(Constants.PRODUCT_NAME));
#endif
            Logger.WriteEntry(LogLevel.Information, "log file: " + FileLogging.GenerateDefaultLogFileName(Constants.PRODUCT_NAME) + " log level :" + Logger.currentLogLevel);

        }
		/// <summary>
		///      Implements the OnBeginShutdown method of the IDTExtensibility2 interface.
		///      Receives notification that the host application is being unloaded.
		/// </summary>
		/// <param term='custom'>
		///      Array of parameters that are host application specific.
		/// </param>
		/// <seealso class='IDTExtensibility2' />
		public void OnBeginShutdown(ref System.Array custom)
		{
            foreach (String key in _InspectorWrappers.Keys)
            {
                _InspectorWrappers.Remove(key);
            }            
		}


        /// <summary>
        /// 
        /// </summary>
        /// <param name="newInspector"></param>        
        private void Inspectors_NewInspector(Inspector newInspector)
        {

            if (newInspector != null)
            {
                InspectorWrapper wrapper = new InspectorWrapper(newInspector);
                _InspectorWrappers.Add(wrapper.GetHashCode(), wrapper);
            }

        }

        /// <summary>
        /// 
        /// </summary>

        private void initializeEventsHandler()
        {
            ServicePointManager.ServerCertificateValidationCallback = new RemoteCertificateValidationCallback(MyCertValidationCb);
            _Inspectors = _OutlookApplication.Inspectors;

            //New Inspector
            _Inspectors.NewInspector += new InspectorsEvents_NewInspectorEventHandler(Inspectors_NewInspector);

            //Context menu
            _OutlookApplication.ItemContextMenuDisplay += new ApplicationEvents_11_ItemContextMenuDisplayEventHandler(applicationObject_ItemContextMenuDisplay);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="bar"></param>
        /// <param name="sel"></param>
        void applicationObject_ItemContextMenuDisplay(CommandBar bar, Selection sel)
        {
            try
            {
                if (sel.Count == 1)
                {
                    if (_OutlookApplication.ActiveExplorer().Selection[1] is AppointmentItem)
                    {
                        AppointmentItem theItem = _OutlookApplication.ActiveExplorer().Selection[1] as AppointmentItem;
                        addButton(bar, theItem);
                    }
                    else if (_OutlookApplication.ActiveExplorer().Selection[1] is ContactItem)
                    {
                        ContactItem theItem = _OutlookApplication.ActiveExplorer().Selection[1] as ContactItem;
                        addButton(bar, theItem);
                    }
                    else if (_OutlookApplication.ActiveExplorer().Selection[1] is MailItem)
                    {
                        MailItem theItem = _OutlookApplication.ActiveExplorer().Selection[1] as MailItem;
                        addButton(bar, theItem);
                    }
                }
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, ex.GetType() + ":"+ ex.Message);
            }

        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="bar"></param>
        /// <param name="item"></param>
        void addButton(CommandBar bar, AppointmentItem item)
        {
            String caption = null;
            if (String.Compare(_OutlookApplication.Session.CurrentUser.Name, item.Organizer) == 0)
            {
                caption  = Constants.CAPTION_START_BUTTON;
            }
            else
            {
                caption = Constants.CAPTION_JOIN_BUTTON;
            }
            
            Utility.addButton(bar, Constants.TAG_START_APPOINTMENTITEM_CONTEXT_MENU + item.EntryID + DateTime.Now.GetHashCode().ToString(), Constants.DESC_START_JOIN_BUTTON, caption, Type.Missing, true, _start_meeting_button_handler, item.EntryID);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="bar"></param>
        /// <param name="item"></param>
        void addButton(CommandBar bar, MailItem item)
        {
            Utility.addButton(bar, Constants.TAG_CALL_MAIL_CONTEXT_MENU + item.EntryID + DateTime.Now.GetHashCode().ToString(), Constants.DESC_CALL_BUTTON, Constants.CAPTION_CALL_CONTEXT_BUTTON, Type.Missing, true, _call_sender_button_handler, item.EntryID);
        }
        /// <summary>
        /// 
        /// </summary>
        /// <param name="bar"></param>
        /// <param name="item"></param>
        void addButton(CommandBar bar, ContactItem item)
        {
            Utility.addButton(bar, Constants.TAG_CALL_CONTACT_CONTEXT_MENU + item.EntryID + DateTime.Now.GetHashCode().ToString(), Constants.DESC_CALL_BUTTON, Constants.CAPTION_CALL_CONTEXT_BUTTON, Type.Missing, true, _call_contact_button_handler, item.EntryID);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="cmdBarbutton"></param>
        /// <param name="cancel"></param>
        public void ItemContext_Click(CommandBarButton cmdBarbutton, ref bool cancel)
        {
            if (_OutlookApplication.ActiveExplorer().Selection.Count != 1) return;
            
            Object item = _OutlookApplication.ActiveExplorer().Selection[1];
            ClickToCalllHandler handler = new ClickToCalllHandler(item);
            handler.start();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="cmdBarbutton"></param>
        /// <param name="cancel"></param>
        private void scsButton_Click(CommandBarButton cmdBarbutton, ref bool cancel)
        {
            try
            {
                _scsConfigDlg.ShowDialog();
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Exception :" + ex.GetType() + " " +  ex.Message + " " + ex.StackTrace);
                MessageBox.Show("Exception:" + ex.Message );
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="certificate"></param>
        /// <param name="chain"></param>
        /// <param name="sslPolicyErrors"></param>
        /// <returns></returns>
        public static bool MyCertValidationCb(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {


#if CHECK_CERTIFICATE            
            if (sslPolicyErrors == SslPolicyErrors.RemoteCertificateChainErrors)
            {
                MessageBox.Show("SCS: Remote Certificate Chain Error!");
                return false;
            }

            
            else if (sslPolicyErrors == SslPolicyErrors.RemoteCertificateNameMismatch)
            {
                SecurityZone z = default(Zone);
                z = Zone.CreateFromUrl(((HttpWebRequest)sender).RequestUri.ToString());
                if ((z.SecurityZone == System.Security.SecurityZone.Intranet | z.SecurityZone == System.Security.SecurityZone.MyComputer))
                {
                    return true;
                }
                else
                {
                    MessageBox.Show("Remote Certificate Name Mismatch");
                    return false;
                }
            }

            if (sslPolicyErrors == SslPolicyErrors.RemoteCertificateNotAvailable)
            {
                MessageBox.Show("Remote Certificate not available");
                return false;
            }

            if (sslPolicyErrors == SslPolicyErrors.None)
            {
                return true;
            }

#endif

            return true;

        }


        #region IRibbonExtensibility Members

        string IRibbonExtensibility.GetCustomUI(string RibbonID)
        {
            if (RibbonID == "Microsoft.Outlook.Mail.Read")
            {
                return customizeGroupName(Properties.Resources.RibbonCall, 2);
            }
            else if (RibbonID == "Microsoft.Outlook.Appointment")
            {
                return customizeGroupName(Properties.Resources.RibbonMeeting,1);
            }
            else if (RibbonID == "Microsoft.Outlook.Contact")
            {
                return customizeGroupName(Properties.Resources.RibbonCall,2);
            }                                     

            return null;
        }

        #endregion

        /// <summary>
        /// 
        /// </summary>
        /// <param name="xmlstr"></param>
        /// <param name="number"></param>
        /// <param name="name"></param>
        /// <returns></returns>
        String customizeGroupName(String xmlstr, int number)
        {
            XmlDocument doc = new XmlDocument();
            doc.LoadXml(xmlstr);

            for (int i = 0; i < number; i++)
            {
                doc.GetElementsByTagName("group")[i].Attributes["label"].InnerXml = Constants.SERVER_FLAG;
            }
            return doc.OuterXml;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="control"></param>
        public void Click2Call(IRibbonControl control)
        {
            ClickToCalllHandler handler = new ClickToCalllHandler(this._OutlookApplication.ActiveInspector().CurrentItem);
            handler.start();
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="control"></param>
        public void SetMeetingLocation(IRibbonControl control)
        {
            bool cancel = false;
            InspectorWrapper.scsmeetingButton_Click(null, ref cancel);
        }


        /// <summary>
        /// 
        /// </summary>
        private void RemoveSCSPopup()
        {
            // If the menu already exists, remove it.
            try
            {
                CommandBarPopup foundMenu = (CommandBarPopup)
                    _OutlookApplication.ActiveExplorer().CommandBars["Tools"].
                    FindControl(MsoControlType.msoControlPopup,
                    System.Type.Missing, Constants.TAG_SCS_CONFIG_BUTTON_IN_TOOLS, true, true);
                if (foundMenu != null)
                {
                    foundMenu.Delete(true);
                }
            }
            catch (System.Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public void AddSCSPopupButton()
        {
            try
            {
                CommandBar toolbar = _OutlookApplication.ActiveExplorer().CommandBars["Tools"];
                _scsPopupDlg = (CommandBarPopup)toolbar.Controls.Add(MsoControlType.msoControlPopup, Type.Missing, Type.Missing, Type.Missing, true);

                _scsPopupDlg.Tag = Constants.TAG_SCS_CONFIG_BUTTON_IN_TOOLS;
                _scsPopupDlg.Caption = Constants.CAPTION_SCS_CONFIG_BUTTON_IN_TOOLS;

                _scsSettingButton = (CommandBarButton)_scsPopupDlg.Controls.Add(MsoControlType.msoControlButton, Type.Missing, Type.Missing, Type.Missing, true);

                _scsSettingButton.Tag = Constants.TAG_SCS_ACCOUNT_SETTING_BUTTON;
                _scsSettingButton.DescriptionText = Constants.DESC_SCS_ACCOUNT_SETTING_BUTTON;
                _scsSettingButton.Caption = Constants.CAPTION_SCS_ACCOUNT_SETTING_BUTTON;
                _scsSettingButton.Style = MsoButtonStyle.msoButtonIconAndCaption;
                _scsSettingButton.Click += new _CommandBarButtonEvents_ClickEventHandler(settingbutton_Click);


                _scsPasscodeButton = (CommandBarButton)_scsPopupDlg.CommandBar.Controls.Add(MsoControlType.msoControlButton, Type.Missing, Type.Missing, Type.Missing, true);

                _scsPasscodeButton.Tag = Constants.TAG_SCS_ACCOUNT_PASSCODE_BUTTON;
                _scsPasscodeButton.DescriptionText = Constants.DESC_SCS_ACCOUNT_PASSCODE_BUTTON;
                _scsPasscodeButton.Caption = Constants.CAPTION_SCS_ACCOUNT_PASSCODE_BUTTON;
                _scsPasscodeButton.Style = MsoButtonStyle.msoButtonIconAndCaption;
                _scsPasscodeButton.Click += new _CommandBarButtonEvents_ClickEventHandler(passwdbutton_Click);


                _scsCallRulesButton = (CommandBarButton)_scsPopupDlg.CommandBar.Controls.Add(MsoControlType.msoControlButton, Type.Missing, Type.Missing, Type.Missing, true);

                _scsCallRulesButton.Tag = Constants.TAG_SCS_CALL_RULES_BUTTON;
                _scsCallRulesButton.DescriptionText = Constants.DESC_SCS_CALL_RULES_BUTTON;
                _scsCallRulesButton.Caption = Constants.CAPTION_SCS_CALL_RULES_BUTTON;
                _scsCallRulesButton.Style = MsoButtonStyle.msoButtonIconAndCaption;
                _scsCallRulesButton.Click += new _CommandBarButtonEvents_ClickEventHandler(_scsCallRulesButton_Click);


                _scsPopupDlg.Visible = true;
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Exception :" + ex.GetType() + " " + ex.Message + " " + ex.StackTrace);
                MessageBox.Show(ex.Message);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Ctrl"></param>
        /// <param name="CancelDefault"></param>
        void _scsCallRulesButton_Click(CommandBarButton Ctrl, ref bool CancelDefault)
        {
            try
            {
                CallRulesWindow wnd = new CallRulesWindow();
                wnd.ShowDialog();
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Exception :" + ex.GetType() + " " + ex.Message + " " + ex.StackTrace);
                MessageBox.Show(ex.Message);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Ctrl"></param>
        /// <param name="CancelDefault"></param>
        void settingbutton_Click(CommandBarButton Ctrl, ref bool CancelDefault)
        {
            scsButton_Click(Ctrl, ref CancelDefault);
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="Ctrl"></param>
        /// <param name="CancelDefault"></param>
        void passwdbutton_Click(CommandBarButton Ctrl, ref bool CancelDefault)
        {
            try
            {
                PassCodeWindow wnd = new PassCodeWindow();
                wnd.ShowDialog();
            }

            catch (System.Exception ex)
            {
                Logger.WriteEntry(LogLevel.Error, "Exception :" + ex.GetType() + " " + ex.Message + " " + ex.StackTrace);
                MessageBox.Show(ex.Message);
            }
        }

        private Microsoft.Office.Interop.Outlook.Application _OutlookApplication;
		private object _addInInstance;

        private Inspectors _Inspectors = null;
        private Hashtable _InspectorWrappers = new Hashtable();

        _CommandBarButtonEvents_ClickEventHandler _scs_button_handler = null;
        _CommandBarButtonEvents_ClickEventHandler _call_contact_button_handler = null;
        _CommandBarButtonEvents_ClickEventHandler _call_sender_button_handler = null;
        _CommandBarButtonEvents_ClickEventHandler _start_meeting_button_handler = null;

        BaseEditDialog _scsConfigDlg = new SettingsDialog();

        CommandBarPopup _scsPopupDlg = null;
        CommandBarButton _scsSettingButton = null;
        CommandBarButton _scsPasscodeButton = null;
        CommandBarButton _scsCallRulesButton = null; 


        IRibbonUI _TheRibbon;        

        public void ribbonLoaded(IRibbonUI ribbon)
        {
            _TheRibbon = ribbon;
        }
	}
}