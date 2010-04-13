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
using System.ComponentModel;
using System.Configuration.Install;
using System.IO;
using System.Collections;
using System.Windows.Forms;
using System.Security;
using System.Security.Policy;
using AddinForOutlook;


namespace AddinForOutlookSetupExt
{
    [RunInstaller(true)]
    public partial class CustomInstaller : Installer
    {
        public CustomInstaller()
        {
            InitializeComponent();
        }

        private void CustomInstaller_AfterInstall(object sender, InstallEventArgs e)
        {
            string serverName = this.Context.Parameters["SERVERNAME"];
            string userName = this.Context.Parameters["USERNAME"];
            string conferenceNumber = this.Context.Parameters["CONFERENCENUMBER"];
            string pinNumber = this.Context.Parameters["PINNUMBER"];
            string isDefaultLoc = this.Context.Parameters["ISDEFAULTLOCATION"];

            Microsoft.Win32.RegistryKey key;

            key = Microsoft.Win32.Registry.CurrentUser.CreateSubKey(Constants.REG_KEY_NAME_ADDIN_SETTING);

            key.SetValue(Constants.REG_ATTR_NAME_CTI_SERVER_NAME, serverName, Microsoft.Win32.RegistryValueKind.String);
            key.SetValue(Constants.REG_ATTR_NAME_USERNAME, userName, Microsoft.Win32.RegistryValueKind.String);
            key.SetValue(Constants.REG_ATTR_NAME_CONFERENCE_NUMBER, conferenceNumber, Microsoft.Win32.RegistryValueKind.String);
            key.SetValue(Constants.REG_ATTR_NAME_CONFERENCE_ACCESS_CODE, pinNumber, Microsoft.Win32.RegistryValueKind.String);

            if (isDefaultLoc == "1")
            {
                key.SetValue(Constants.REG_ATTR_NAME_ISDEFAULTLOCATION, 1, Microsoft.Win32.RegistryValueKind.DWord);
            }
            else
            {
                key.SetValue(Constants.REG_ATTR_NAME_ISDEFAULTLOCATION, 0, Microsoft.Win32.RegistryValueKind.DWord);
            }

            key.Close();
        }

        /// <summary>
        /// 
        /// </summary>
        /// 
        public override void Install(System.Collections.IDictionary stateSaver)
        {            
            base.Install(stateSaver);

            try
            {                
                // Find the machine policy level
                PolicyLevel machinePolicyLevel = null;
                System.Collections.IEnumerator policyHierarchy = SecurityManager.PolicyHierarchy();

                while (policyHierarchy.MoveNext())
                {
                    PolicyLevel level = (PolicyLevel)policyHierarchy.Current;
                    if (level.Label == "Machine")
                    {
                        machinePolicyLevel = level;
                        break;
                    }
                }

                if (machinePolicyLevel == null)
                {
                    throw new ApplicationException(
                        "Could not find Machine Policy level. Code Access Security " +
                        "is not configured for this application."
                        );
                }

                // Get the installation directory of the current installer
                string assemblyPath = this.Context.Parameters["assemblypath"];
                string installDirectory = assemblyPath.Substring(0, assemblyPath.LastIndexOf("\\"));

                if (!installDirectory.EndsWith(@"\"))
                    installDirectory += @"\";

                installDirectory += "*";

                //Create a code group with a new FullTrust permission set
                PolicyStatement policyStatement = new PolicyStatement(new NamedPermissionSet("FullTrust"));
                CodeGroup codeGroup = new UnionCodeGroup(new UrlMembershipCondition(installDirectory), policyStatement);
                codeGroup.Description = "Permissions for Outlook Addin";
                codeGroup.Name = "Outlook Addin";

                machinePolicyLevel.RootCodeGroup.AddChild(codeGroup);
                SecurityManager.SavePolicy();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        } 
    }
}