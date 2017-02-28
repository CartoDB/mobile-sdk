/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_BILLBOARDSORTER_H_
#define _CARTO_BILLBOARDSORTER_H_

#include <memory>
#include <vector>

namespace carto {
    class BillboardDrawData;
    class ViewState;
    
    class BillboardSorter {
    public:
        BillboardSorter();
        virtual ~BillboardSorter();
    
        void clear();
    
        void add(const std::shared_ptr<BillboardDrawData>& drawData);
    
        void sort(const ViewState& viewState);
    
        const std::vector<std::shared_ptr<BillboardDrawData> >& getSortedBillboardDrawDatas() const;
    
    private:
        std::vector<std::shared_ptr<BillboardDrawData> > _billboardDrawDatas;
    };
    
}

#endif
