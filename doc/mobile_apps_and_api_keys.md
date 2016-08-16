# Mobile Apps and API Keys

You must register your mobile applications under your CARTO account settings. Once an app is added, you can retrieve the mobile app API Key and manage its settings.

## Register a Mobile APP

The following procedure describes how to register mobiles apps under your account settings. While the number of mobile apps is unlimited, application quota consumption rules are enforced based on your account plan. [Contact us](mailto:sales@carto.com) if you have questions about your mobile account options.

1. Access [Your API Keys](/docs/carto-editor/your-account/#api-key) from the CARTO Dashboard

  <span class="wrap-border"><img src="{{ site.baseurl }}/img/layout/mobile/access_mobile_api_key.jpg" alt="Access mobile apps from API keys" /></span>

  The API Key page opens, displaying options for _CARTO_ or _Mobile Apps_. 

2. Click _Mobile apps_ to add mobile applications to your account

  <span class="wrap-border"><img src="{{ site.baseurl }}/img/layout/mobile/no_registered_apps.jpg" alt="No registered mobile apps" /></span>

3. Click _NEW APPLICATION_

  The new application settings appear.

4. Enter options for the mobile application
 
  <span class="wrap-border"><img src="{{ site.baseurl }}/img/layout/mobile/new_mobile_app_settings.jpg" alt="New mobile app settings" /></span>

  The following mobile application settings are available: 

  Mobile Application Settings | Description
  --- | ---
  App icon | Select an icon to represent your mobile application in your account
  Name | An informative name of the mobile application in your account
  Description | A description about the mobile application in your account
  Platform | Identifies the type of mobile app platform. Some options may not be available, based on your account plan<br/><br/>**Note:** Once a mobile application is saved, you cannot edit the Platform setting. As an alternative, you can [delete](#delete-a-mobile-app) the application and recreate it with new settings.<br/><br/>**Tip:** When you select a _Platform_, the _Application ID_ displays more details about what identifiers, or packages, are required.<br/><br/><img src="{{ site.baseurl }}/img/layout/mobile/platform_appid.jpg" alt="Platform App ID hint" /> 
  Application ID | The identification of the mobile app in the App store that is required for development with the Mobile SDK.<br/><br/>The following application ids identify the Platform selected:<br/><br/>- [**Android**](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#android-implementation): requires the AndroidManifest.xml package<br/><br/>- [**iOS**](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#ios-implementation): requires a Bundle identifier. You can find it in the project properties of your iOS package<br/><br/>- [**Xamarin Android**](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#xamarin-android-and-ios-implementation): requires the AndroidManifest.xml package<br/><br/>- [**Xamarin iOS](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#xamarin-ios-app)**: requires a Bundle identifier. You can find it in the project properties of your Xamarin iOS package<br/><br/>- [**Windows Phone**](/docs/carto-engine/mobile-sdk/mobile-platforms-implementation/#windows-phone-implementation): Requires the package name from the Windows package.appmanifest 
  App type | Identifies the type of mobile application, which are subject to different quota limitations<br/><br/>- **Open**: Adds and counts mobile apps users towards global usage quotas, ranging from 10K to 500k users, depending on your account plan. This app type is open to the public and available in public app stores<br/><br/>- **Dev**: Select this app type when you are developing and testing your mobile app. Dev is available for all account plans and allows each mobile app to have up to five users. Dev apps do not appear in public app stores<br/><br/>_**Note:** Once you are satisfied with your mobile development, you can [promote a Dev app](#promoting-a-dev-app) to another app type._<br/><br/>- **Private**: Available for Enterprise accounts only, allows organizations to publish their apps internally and share quota limitations, up to 750 users. Private apps are not published to public app stores
 
5. Click _SAVE CHANGES_

  The Mobile apps page refreshes, displaying the added mobile application and the features enabled. 

## Access your Mobile API Key

Once your mobile apps are registered for your account, you can retrieve the API Key for each application. This is useful when you need to copy and paste the API Key for mobile development.

1. From the _Mobile apps_ section of your API Keys, click a mobile application name

  <span class="wrap-border"><img src="{{ site.baseurl }}/img/layout/mobile/registered_mobile_app.jpg" alt="Registered mobile apps" /></span>

  The application settings appear, and include the license key and amount of quota for the application. 

2. Copy the API Key from the _License key_ section

  The following example displays the API Keys section for a selected mobile application.  Note, the API Key in the example image is blurred out for privacy.

  <span class="wrap-border"><img src="{{ site.baseurl }}/img/layout/mobile/example_settings.jpg" alt="Example of mobile app settings" /></span>

3. Click the back navigation arrow, located next to the app name, to return Mobile apps section of your API Keys

### License Key Rules

Mobile App API Keys cannot be regenerated manually, but are automatically regenerated in the following scenarios.

- If you [request a new API Key](/docs/carto-editor/your-account/#api-key) for your CARTO API's, the Mobile apps license key is also automatically regenerated

- If you [upgrade](/docs/carto-editor/your-account/#billing) your account plan that includes additional mobile features, the Mobile apps license key is automatically regenerated

- If you change the _App type_ from **Dev** to Open or Private, the Mobile apps license key is regenerated

You will receive a notification when mobile api keys are regenerated.

## Promoting a Dev App

A Dev mobile app enables you to develop and test your mobile application. Only a developer mobile app can be promoted to another app type through the Mobile apps settings.

**Note:** Only the Dev app type can be promoted. You cannot change the mobile app type if it is Open or Private. As an alternative, you can delete the application and [recreate it](#register-a-mobile-app) with the new app type.

1. From the _Mobile apps_ section of your API Keys, click a mobile application name

  The application settings appear. 

2. Click _Delete this application_ from the bottom of the app settings

  <span class="wrap-border"><img src="{{ site.baseurl }}/img/layout/mobile/change_app_type.jpg" alt="Promote dev app to another app type" /></span>

3. Click _SAVE CHANGES_

  A confirmation dialog appears and notifies you that the mobile app license key will also change.

4. Click _CHANGE APPLICATION TYPE_

  The selected mobile app settings refresh, and the license key is regenerated.

5. Copy and paste the regenerated API Key, as needed for your mobile app

6. Click the back navigation arrow, located next to the app name, to return Mobile apps section of your API Keys

## Delete a Mobile App

Once a mobile application is saved, you cannot edit the Platform setting. As an alternative, you can delete the application and [recreate it](#register-a-mobile-app) with new settings.

1. From the _Mobile apps_ section of your API Keys, click a mobile application name

  The application settings appear, and include the license key and amount of quota for the application. 

2. Click _Delete this application_ from the bottom of the app settings

  <span class="wrap-border"><img src="{{ site.baseurl }}/img/layout/mobile/delete_application.jpg" alt="Delete mobile app" /></span>

3. Click the back navigation arrow, located next to the app name, to return Mobile apps section of your API Keys
