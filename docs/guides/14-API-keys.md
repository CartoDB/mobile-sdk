## API Keys

### Registering Mobile App

The following procedure describes how to register mobile apps under your account settings. While the number of mobile apps is unlimited, application quota consumption rules are enforced based on your account plan. [Contact us](mailto:sales@carto.com) if you have questions about your mobile account options.

1. Access your [Account settings](https://carto.com/docs/carto-editor/your-account/#how-to-access-your-account-options) from the CARTO Dashboard. Your profile information appears. 
    <span class="wrap-border"><img src="../../img/avatar.gif" alt="Access mobile apps from API keys" /></span>
2. Click _API keys_. The API key page opens, displaying options for _CARTO_ or _Mobile Apps_.
3. Click _Mobile apps_ to add mobile applications to your account

**Note:** If this option is not visible from your account settings, [contact us](mailto:sales@carto.com) to ensure that Mobile SDK features are enabled for your account.

<span class="wrap-border"><img src="../../img/no_registered_apps.jpg" alt="No registered mobile apps" /></span>

4. Click _NEW APPLICATION_ The new application settings appear.
5. Enter options for the mobile application
 
<span class="wrap-border"><img src="../../img/new_mobile_app_settings.jpg" alt="New mobile app settings" /></span>

The following mobile application settings are available: 

Mobile Application Settings | Description
--- | ---
App icon | Select an icon to represent your mobile application in your account
Name | An informative name of the mobile application in your account
Description | A description about the mobile application in your account
Platform | Identifies the type of mobile app platform. Some options may not be available, based on your account plan<br/><br/>Once a mobile application is saved, you cannot edit the Platform setting. As an alternative, you can [delete](#deleting-an-app) the application and recreate it with new settings.<br/><br/> When you select a _Platform_, the _Application ID_ displays more details about what identifiers, or packages, are required.<br/><br/><img src="../../img/platform_appid.jpg" alt="Platform App ID hint" /> 
Application ID | The identification of the mobile app in the App store that is required for development with the Mobile SDK.<br/><br/>The following application ids identify the Platform selected:<br/><br/>- **Android**: *package* from the AndroidManifest.xml file or *applicationId* in build.gradle<br/><br/>- **iOS**: Bundle Identifier. You can find it in the project properties in Xcode<br/><br/>- **Xamarin Android**: *package* from the AndroidManifest.xml file<br/><br/>- **Xamarin iOS**: Bundle Identifier from the project Info.plist<br/><br/>- **Windows Phone (UWP)**: UUID from the Windows package.appmanifest 
App type | Identifies the type of mobile application, which are subject to different quota limitations<br/><br/>- **Open**: This app type is open to the public and is available as free download in public app stores<br/><br/>- **Private**: Available for Enterprise accounts only, allows organizations to publish their apps internally and have paid app use cases. 


6. Click _SAVE CHANGES_ .The Mobile apps page refreshes, displaying the added mobile application and the features enabled. 

### Access your API Key

Once your mobile apps are registered for your account, you can retrieve the API Key for each application. This is useful when you need to copy and paste the API Key for mobile development.

1. From the _Mobile apps_ section of your API Keys, click a mobile application name. The application settings appear, and include the license key and amount of quota for the application.

 <span class="wrap-border"><img src="../../img/registered_mobile_app.jpg" alt="Registered mobile apps" /></span>
2. Copy the API Key from the _License key_ section. The following example displays the API Keys section for a selected mobile application.  Note, the API Key in the example image is blurred out for privacy.

 <span class="wrap-border"><img src="../../img/example_settings.jpg" alt="Example of mobile app settings" /></span>
3. Click the back navigation arrow, located next to the app name, to return Mobile apps section of your API Keys

### API Key Rules

Mobile App API Keys cannot be regenerated manually, but are automatically regenerated in the following scenarios.

- If you [upgrade](/docs/carto-editor/your-account/#billing) your account plan that includes additional mobile features, the Mobile apps license key is automatically regenerated
- If you change the _App type_ the Mobile apps license key is regenerated

You will receive a notification when mobile api keys are regenerated.

### Deleting an App

Once a mobile application is saved, you cannot edit the Platform setting. As an alternative, you can delete the application and recreate it with new settings.

1. From the _Mobile apps_ section of your API Keys, click a mobile application name.  The application settings appear, and include the license key and amount of quota for the application. 
2. Click _Delete this application_ from the bottom of the app settings

 <span class="wrap-border"><img src="../../img/delete_application.jpg" alt="Delete mobile app" /></span>
3. Click the back navigation arrow, located next to the app name, to return Mobile apps section of your API Keys
