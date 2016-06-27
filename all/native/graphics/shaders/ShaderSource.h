#ifndef _CARTO_SHADERSOURCE_H_
#define _CARTO_SHADERSOURCE_H_

#include <string>

namespace carto {

    class ShaderSource {
    public:
        ShaderSource(const std::string& name, const std::string* vertSource, const std::string* fragSource) :
            _name(name),
            _vertSource(vertSource),
            _fragSource(fragSource)
        {
        }
    
        const std::string& getName() const { return _name; }
    
        const std::string* getVertSource() const { return _vertSource; }
        const std::string* getFragSource() const { return _fragSource; }
    
    private:
        std::string _name;
    
        const std::string* _vertSource;
        const std::string* _fragSource;
    };
    
}

#endif

