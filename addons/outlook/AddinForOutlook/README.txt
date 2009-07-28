Release Notes of sipXecs/SCS Add in 1.0 for Outlook
July 27, 2009

The sipXecs/SCS “add in” for Outlook enables users from outlook to control a user’s desktop phone registered with a sipX/SCS server.  For example, by simply click a button, an outlook user could call a mail sender, a contact or call to a conference without memorizing/typing all phone numbers.

Supported Outlook version
Outlook 2007

Prerequisite
1)	Microsoft .Net 2.0
2)	Outlook 2007 
3)	Microsoft  Installer 3.1 (for installation)

Basic functionality
1)	Create a meeting/appointment with using sipX conference as default "Location". 
An outlook user could configure the “add in” through an option under "tools->SCS configuration->Settings" menu, a user could choose whether the default meeting/appointment location is using SCS conference or not.  If the option is checked, every time a new meeting/appointment is created, the location field is default set to use SCS conference facility. 

2)	Set a meeting/appointment “location" field by simply click a button (Meeting@SCS).
There is a button (Meeting@SCS) available on the "new appointment" Ribbon UI under “SCS” group.  Users can set the Location to be using SCS facility by just a click.

3)	Call to conference from appointment item’s context menu
Whenever an appointment item is right clicked from the calendar folder, there is a "Start” or “Join" button appears on the context menu based on whether the outlook user is the meeting organizer or not.  A user could initiate the meeting or Join the meeting by just a click.

4)	Call to conference from an opened item
When a meeting item is opened from the calendar folder, there is a "Start” button in a group “SCS” on the Ribbon UI.  A user could initiate the meeting or Join the meeting by just a click.

5)	Call a contact 
When a contact item is opened or right clicked, a user could call the contact by clicking the "SCS call" option from the context menu or the “call” button in the “SCS” group on the Ribbon UI

6)	Call a mail sender
When a single mail item is opened or right clicked, a user could call the sender by clicking the "call" option in the “SCS” group  on the Ribbon UI/or “SCS call” button on the Context-Menu.

Configuration

A "SCS  configuration" option is added into "tools" to allow user to 
•	Provision settings such as server IP/host,  SIP user account,  conference number, conference access code and set SCS as default conference location
•	Set/Change User account PIN number for current outlook session
Notes: For security purpose, the typed PIN is only available during current outlook session. After outlook exits, the typed PIN is removed. 
•	Provision calling rules - Users could define a global prefix to remove and add  respectively for the called number

Logging
Via modifying the following registry attribute in windows registry, 
"Software\\Microsoft\\Office\\Outlook\\Addins\\SCSAddin4Outlook.Connect\\custom\\log_level”, logging level could be defined to the following value:
•	0 – information
•	1 – Waning
•	2 – Error  this is the default level
•	3 – Turn off – logging is turned off

Notes:  after the attribute is modified, an outlook restart is required for the new value to taking into effective.

The log file is named in the fashion of SCSAddinForOutlook_<month>_<day>_<year>.log.  And it is located under the office/outlook’s installation directory for example, C:\Program Files\Microsoft Office\Office12. Check your own PC installation for its value.

Branding Customization

The “SCS” branding showing in the GUI by default could be customized to different value via 
"Software\\Microsoft\\Office\\Outlook\\Addins\\SCSAddin4Outlook.Connect\\custom\\server_branding”.

Notes: after the attribute is modified, an outlook restart is required for the new value to taking into effective.

