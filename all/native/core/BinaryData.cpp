#include "BinaryData.h"

#include <algorithm>
#include <sstream>

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

    bool BinaryData::operator ==(const BinaryData& data) const {
        if (_dataPtr->size() != data._dataPtr->size()) {
            return false;
        }
        return std::equal(_dataPtr->begin(), _dataPtr->end(), data._dataPtr->begin());
    }

    bool BinaryData::operator !=(const BinaryData& data) const {
        return !(*this == data);
    }

    int BinaryData::hash() const {
        return static_cast<int>(std::hash<std::string>()(std::string(reinterpret_cast<const char*>(_dataPtr->data()), _dataPtr->size())));
    }

    std::string BinaryData::toString() const {
        std::stringstream ss;
        ss << "BinaryData [size=" << _dataPtr->size() << "]";
        return ss.str();
    }

}
