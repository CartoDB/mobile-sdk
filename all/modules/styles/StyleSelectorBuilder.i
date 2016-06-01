#ifndef _STYLESELECTORBUILDER_I
#define _STYLESELECTORBUILDER_I

%module StyleSelectorBuilder

!proxy_imports(carto::Style, styles.Style, styles.StyleSelector)

%{
#include "styles/StyleSelectorBuilder.h"
#include <memory>
%}

%include <std_shared_ptr.i>
%include <cartoswig.i>

%import "styles/Style.i"
%import "styles/StyleSelector.i"

!shared_ptr(carto::StyleSelectorBuilder, styles.StyleSelectorBuilder)

!objc_rename(addFilterRule) carto::StyleSelectorBuilder::addRule(const std::string&, const std::shared_ptr<Style>&);
%ignore carto::StyleSelectorBuilder::addRule(const std::shared_ptr<StyleSelectorRule>&);

%include "styles/StyleSelectorBuilder.h"

#endif
