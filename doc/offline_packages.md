# Offline Map Packages

Offline map packages enable mobile maps to work without an Internet connection and are downloaded to your local mobile app. There are several map packages that can be downloaded from CARTO for offline use. Your must use the **Package Manager** API to download the packages. _Offline map packages are only available to certain account plans._ [Contact us](mailto:support@carto.com) if you have questions about your Mobile SDK plan.

The following offline packages are available:

-  **Continent/Sub-continent/Country/Country Region**, for general geographical hierarchies

-  **Sub-continent**, only used in Europe, Asia, Africa and North America

-  **Country Region** are used in US, Germany, UK, Spain, Russia and Canada. Spain and UK also have their own individual, offline packages

-  **By Language** - offline packages are also available in following languages: **English (en), German, (de), French (fr), Italian (it), Chinese (zh), Spanish (es), Russian (ru) and Estonian (et)**. [Contact us](mailto:support@carto.com) if you need another language

## CARTO OpenStreetMap Packages

-  When initializing the Package Manager API, use `nutiteq.osm` as the source ID
-  For map data, see the [OpenStreetMap](http://www.openstreetmap.org) contributors, ODbL license

{% highlight javascript %}
<script type="text/javascript">
\$(document).ready(function() {\

\$.get(‘//d2izt4z5iclshv.cloudfront.net/mappackages/v1/nutiteq.osm/packages.json’,
function(data) {

var p\_name = ’’;\
 var obj = \$.parseJSON(data);\
 var sorted = \_.sortBy(obj.packages, function(item) {\
 return (\[item.metainfo.name\_en\]);\
 });

p\_name =
‘<table class="table table-striped table-bordered"><tr><th style="padding:5px;">Package
name</th><th></th><th style="padding:5px;">ID</th><th></th><th style="padding:5px;">Size
(MB)</th><th></th><th style="padding:5px;">Ver</th><th></th></tr>’;\
 \$.each(sorted, function () {\
 p\_name += “<tr>”;\
 p\_name *= ’<td style="padding:5px;">’* this.metainfo.name\_en +
“<td/>”;\
 p\_name *= ’<td style="padding:5px;">’* this.id + “<td/>”;\
 p\_name *= ’<td style="padding:5px;">’*
(this.size/(1024\*1024)).toFixed(1).replace(“.0”, “”) + “<td/>”;\
 p\_name *= ’<td style="padding:5px;">’* this.version + “<td/>”;\
 p\_name += “</tr>”;\
 });\
 p\_name += “</table>”;\
 \$(‘\#packages’).html(p\_name);\
 }, ‘text’);\
 });

</script>
<div id="packages">
</div>
{% endhighlight %}
