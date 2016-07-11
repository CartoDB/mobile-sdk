/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORDATA_H_
#define _CARTO_VECTORDATA_H_

#include <memory>
#include <mutex>
#include <vector>

namespace carto {
    class VectorElement;
    
    /**
     * A wrapper class for vector element data.
     */
    class VectorData {
    public:
        /**
         * Constructs a VectorData object from a list of vector elements.
         * @param elements The list of vector elements.
         */
        VectorData(const std::vector<std::shared_ptr<VectorElement> >& elements);
        virtual ~VectorData();
        
        /**
         * Returns the list of vector elements.
         * @return The list of vector elements.
         */
        const std::vector<std::shared_ptr<VectorElement> >& getElements() const;
        
    private:
        const std::vector<std::shared_ptr<VectorElement> > _elements;
        mutable std::mutex _mutex;
    };

}

#endif
