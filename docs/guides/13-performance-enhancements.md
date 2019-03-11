## Performance enhancements

Here are some hints how to make your map faster. It is useful if you have bigger datasets.

### LocalVectorDataSource Performance

You can define the `spatialIndexType` of the `LocalVectorDataSource` to improve the performance of how a 3D model loads, by defining a spatial index to the DataSource. By default, no spatial index is defined. The following rules apply:

-  Do not define a spatial index if the number of elements is small, below ~1000 points or markers (or even less if you have complex lines and polygons). _This is the default option_

-  Apply `NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE` as the index type if there are a larger number of elements 

The advantage of defining a spatial index is that CPU usage decreases for large number of objects, improving the map performance of panning and zooming. However, displaying overlays may slightly delay the map response, as the spatial index is not loaded immediately when your move the map, it only moves after some hundred milliseconds. 

The overall maximum number of objects on map is limited to the RAM available for the app. Systems define several hundred MB for iOS apps, and closer to tens of MB for Android apps, but it depends on the device and app settings (as well as the density of the data). It is recommended to test your app with the targeted mobile platform and full dataset for the actual performance. 

The following code describes how to adjust the `LocalVectorDataSource` performance by defining a spatial index:

<div class="js-tabs-mobilesdk">
   <ul class="tab-navigation">
     <li class="tab-navigationItem">
       <a href="#tab-java">Java</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-csharp">C#</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-objectivec">Objective-C</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-swift">Swift</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-kotlin">Kotlin</a>
     </li>
   </ul>

  <div id="tab-java">
    {% highlight java linenos %}
    LocalVectorDataSource vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LOCAL_SPATIAL_INDEX_TYPE_KDTREE);
    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}
    var vectorDataSource2 = new LocalVectorDataSource(proj, LocalSpatialIndexType.LocalSpatialIndexTypeKdtree);
    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}
    NTLocalVectorDataSource* vectorDataSource2 =
        [[NTLocalVectorDataSource alloc] initWithProjection:proj
                                         spatialIndexType: NTLocalSpatialIndexType::NT_LOCAL_SPATIAL_INDEX_TYPE_KDTREE];
    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}
    let vectorDataSource2 = NTLocalVectorDataSource(projection: projection, spatialIndexType: NTLocalSpatialIndexType.LOCAL_SPATIAL_INDEX_TYPE_KDTREE)
    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}
    val vectorDataSource2 = LocalVectorDataSource(projection, LocalSpatialIndexType.LOCAL_SPATIAL_INDEX_TYPE_KDTREE)
    {% endhighlight %}
  </div>
  
</div>

<br/><br/>**Note:** If you have **very complex lines or polygons**, this creates objects with high numbers (more than hundreds of points per object) of vertexes. For example, the GPS traces for long periods of time, recording a point every second or so. 

Spatial indexing will not help in this case, as you need to show the whole track on the screen. In this case, apply the **automatic line simplification** parameter for the `LocalVectorDataSource`. This reduces the number of polygon and line points, while maintaining the original object shape. 

**Tip:** Automatic simplification is zoom-dependent. When a map is zoomed out, more aggressive simplification is used. When you zoom in, less simplification is applied until you see the original details.

There are several ways to simplify geometries for automatic simplification by using the following code. Note that simplification is applied in two passes:

- First pass applies fast Radial Distance vertex rejection
- Second pass applies Ramer-Douglas-Peuckerworst algorithm (with the worst case quadratic complexity)

All this is can be applied with just one line of code - as shown in the following example (under iOS) for minimal 1-pixel simplification, the result is invisible but effects are significant, depending on your DataSource:

<div class="js-tabs-mobilesdk">
   <ul class="tab-navigation">
     <li class="tab-navigationItem">
       <a href="#tab-java">Java</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-csharp">C#</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-objectivec">Objective-C</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-swift">Swift</a>
     </li>
     <li class="tab-navigationItem">
       <a href="#tab-kotlin">Kotlin</a>
     </li>
   </ul>

  <div id="tab-java">
    {% highlight java linenos %}
    vectorDataSource2.setGeometrySimplifier(new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f));
    {% endhighlight %}
  </div>

  <div id="tab-csharp">
    {% highlight csharp linenos %}
    vectorDataSource2.GeometrySimplifier = new DouglasPeuckerGeometrySimplifier(1.0f / 320.0f);
    {% endhighlight %}
  </div>

  <div id="tab-objectivec">
    {% highlight objc linenos %}
    [vectorDataSource2 setGeometrySimplifier:
      [[NTDouglasPeuckerGeometrySimplifier alloc] initWithTolerance: 1.0f / 320.0f]];
    {% endhighlight %}
  </div>

  <div id="tab-swift">
    {% highlight swift linenos %}
    vectorDataSource2?.setGeometrySimplifier(NTDouglasPeuckerGeometrySimplifier(tolerance: 1.0 / 320.0))
    {% endhighlight %}
  </div>

  <div id="tab-kotlin">
    {% highlight kotlin linenos %}
    vectorDataSource2.geometrySimplifier = DouglasPeuckerGeometrySimplifier(1.0f / 320.0f)
    {% endhighlight %}
  </div>
    
</div>

The automatic simplification makes renderingfaster with some additional computation on the CPU.