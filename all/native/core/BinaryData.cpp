#include "BinaryData.h"

namespace carto {

    BinaryData::BinaryData() :
    	_dataPtr(std::make_shared<std::vector<unsigned char> >())
    {
    }

    BinaryData::BinaryData(std::vector<unsigned char> data) :
    	_dataPtr(std::make_shared<std::vector<unsigned char> >(std::move(data)))
    {
    }
    
    BinaryData::BinaryData(const unsigned char* data, std::size_t size) :
    	_dataPtr(std::make_shared<std::vector<unsigned char> >(data, data + size))
    {
    }
    
    BinaryData::~BinaryData() {
    }

    bool BinaryData::empty() const {
        return _dataPtr->empty();
    }

    std::size_t BinaryData::size() const {
        return _dataPtr->size();
    }

    const unsigned char* BinaryData::data() const {
        return _dataPtr->data();
    }

    std::shared_ptr<std::vector<unsigned char> > BinaryData::getDataPtr() const {
        return _dataPtr;
    }

}
