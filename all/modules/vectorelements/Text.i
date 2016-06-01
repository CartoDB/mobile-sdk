#ifndef _TEXT_I
#define _TEXT_I

%module Text

!proxy_imports(carto::Text, core.MapPos, graphics.Bitmap, geometry.Geometry, styles.TextStyle, vectorelements.Label)

%{
#include "vectorelements/Text.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/TextStyle.i"
%import "vectorelements/Label.i"

!polymorphic_shared_ptr(carto::Text, vectorelements.Text)

%attributestring(carto::Text, std::string, Title, getText, setText)
%csmethodmodifiers carto::Text::Style "public new";
!attributestring_polymorphic(carto::Text, styles.TextStyle, Style, getStyle, setStyle)

%include "vectorelements/Text.h"

#endif
