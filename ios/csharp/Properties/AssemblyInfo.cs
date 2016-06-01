using System.Reflection;
using System.Runtime.CompilerServices;
using ObjCRuntime;

// Information about this assembly is defined by the following attributes.
// Change them to the values specific to your project.

[assembly: AssemblyTitle ("CartoMobileSDK.iOS")]
[assembly: AssemblyDescription ("Carto Mobile SDK for iOS")]
[assembly: AssemblyConfiguration ("")]
[assembly: AssemblyCompany ("CartoDB")]
[assembly: AssemblyProduct ("")]
[assembly: AssemblyCopyright ("(c) CartoDB 2016, All rights reserved")]
[assembly: AssemblyTrademark ("")]
[assembly: AssemblyCulture ("")]

// The assembly version has the format "{Major}.{Minor}.{Build}.{Revision}".
// The form "{Major}.{Minor}.*" will automatically update the build and revision,
// and "{Major}.{Minor}.{Build}.*" will update just the revision.

[assembly: AssemblyVersion ("1.0.*")]
[assembly: LinkWith("libcarto_mobile_sdk.a", LinkTarget.ArmV7|LinkTarget.ArmV7s|LinkTarget.Arm64|LinkTarget.Simulator|LinkTarget.Simulator64, ForceLoad=true, IsCxx=true, Frameworks="OpenGLES GLKit UIKit CoreGraphics CoreText Foundation")]

// The following attributes are used to specify the signing key for the assembly,
// if desired. See the Mono documentation for more information about signing.

//[assembly: AssemblyDelaySign(false)]
//[assembly: AssemblyKeyFile("")]

