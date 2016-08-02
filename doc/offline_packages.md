Offline Map packages
====================

Following map packages can be downloaded from Carto map service for
offline use. Your app needs to use **Package Manager** API to get the
list, download and consume the packages.

Package names:

-   There is general convention to provide geographical hierarchy and
    avoid too long lists and big packages:
    **Continent/Sub-continent/Country/Country Region**
-   **Sub-continent** part is used only in Europe, Asia, Africa and
    North America
-   **Country Region** division is used in US, Germany, UK, Spain,
    Russia and Canada. Spain and UK are also available as
    single package.
-   Names are available in following languages: **English (en), German
    (de), French (fr), Italian (it), Chinese (zh), Spanish (es),
    Russian (ru) and Estonian (et)**. These are same languages what we
    currently support as map language. Contact us if you need
    another language.

### Carto OpenStreetMap Packages

-   Use source ID: **nutiteq.osm** when initializing Package Manager
-   Map data: [OpenStreetMap](http://www.openstreetmap.org)
    contributors, ODbL license.

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

