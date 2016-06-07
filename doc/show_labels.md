# Show/hide labels for map clicks

MapEventListener, has special functionality: a small label text indicating info about clicked object, as a "Label". This is implemented in following way:

* If you click on an object, then it creates another map object:  **BalloonPopup** with a text from **metadata** of the clicked object. This is why in samples we add special Metadata field value. You can use metadata value directly, or use **object unique ID** as metadata, and then use database to query details about clicked object. 
* When user clicks new location on map, or object, then previous balloon is deleted, and new one is opened in new location

The BalloonPopup is added to a DataSource, which is linked to already existing Layer on map. We use same datasource what we use for other vector elements on map.

See [Event Listener guide](/guides/events) for sample code.