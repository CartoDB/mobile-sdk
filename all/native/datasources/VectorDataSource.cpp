#include "VectorDataSource.h"
#include "graphics/ViewState.h"
#include "vectorelements/VectorElement.h"
#include "utils/Log.h"

#include <algorithm>

namespace carto {
    
    VectorDataSource::VectorDataSource(const std::shared_ptr<Projection>& projection) :
        _projection(projection),
        _onChangeListeners(std::make_shared<std::vector<std::shared_ptr<OnChangeListener> > >()),
        _onChangeListenersMutex()
    {
    }
        
    VectorDataSource::~VectorDataSource() {
    }
        
    std::shared_ptr<Projection> VectorDataSource::getProjection() const {
        return _projection;
    }

    float VectorDataSource::calculateGeometrySimplifierScale(const ViewState& viewState) const {
        float x0 = viewState.getWidth() * 0.5f;
        float y0 = viewState.getHeight() * 0.5f;
        MapPos p0 = viewState.screenToWorldPlane(ScreenPos(x0, y0));
        MapPos p1 = viewState.screenToWorldPlane(ScreenPos(x0 + 1.0f, y0));
        MapVec dp = _projection->fromInternal(p1) - _projection->fromInternal(p0);
        return static_cast<float>(dp.length());
    }
    
    void VectorDataSource::notifyElementsChanged() {
        std::shared_ptr<std::vector<std::shared_ptr<OnChangeListener> > > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        for (const std::shared_ptr<OnChangeListener>& listener : *onChangeListeners) {
            listener->onElementsChanged();
        }
    }
    
    void VectorDataSource::registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            auto onChangeListeners = std::make_shared<std::vector<std::shared_ptr<OnChangeListener> > >(*_onChangeListeners);
            onChangeListeners->push_back(listener);
            _onChangeListeners = onChangeListeners;
        }
    }
    
    void VectorDataSource::unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener) {
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            auto onChangeListeners = std::make_shared<std::vector<std::shared_ptr<OnChangeListener> > >(*_onChangeListeners);
            onChangeListeners->erase(std::remove(onChangeListeners->begin(), onChangeListeners->end(), listener), onChangeListeners->end());
            _onChangeListeners = onChangeListeners;
        }
    }
        
    void VectorDataSource::notifyElementAdded(const std::shared_ptr<VectorElement>& element) {
        std::shared_ptr<std::vector<std::shared_ptr<OnChangeListener> > > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        attachElement(element);
        for (const std::shared_ptr<OnChangeListener>& listener : *onChangeListeners) {
            listener->onElementAdded(element);
        }
    }
    
    void VectorDataSource::notifyElementChanged(const std::shared_ptr<VectorElement>& element) {
        std::shared_ptr<std::vector<std::shared_ptr<OnChangeListener> > > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        for (const std::shared_ptr<OnChangeListener>& listener : *onChangeListeners) {
            listener->onElementChanged(element);
        }
    }
    
    void VectorDataSource::notifyElementRemoved(const std::shared_ptr<VectorElement>& element) {
        std::shared_ptr<std::vector<std::shared_ptr<OnChangeListener> > > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        detachElement(element);
        for (const std::shared_ptr<OnChangeListener>& listener : *onChangeListeners) {
            listener->onElementRemoved(element);
        }
    }
        
    void VectorDataSource::notifyElementsAdded(const std::vector<std::shared_ptr<VectorElement> >& elements) {
        std::shared_ptr<std::vector<std::shared_ptr<OnChangeListener> > > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        for (const std::shared_ptr<VectorElement>& element : elements) {
            attachElement(element);
        }
        for (const std::shared_ptr<OnChangeListener>& listener : *onChangeListeners) {
            listener->onElementsAdded(elements);
        }
    }
    
    void VectorDataSource::notifyElementsRemoved(const std::vector<std::shared_ptr<VectorElement> >& elements) {
        std::shared_ptr<std::vector<std::shared_ptr<OnChangeListener> > > onChangeListeners;
        {
            std::lock_guard<std::mutex> lock(_onChangeListenersMutex);
            onChangeListeners = _onChangeListeners;
        }
        for (const std::shared_ptr<VectorElement>& element : elements) {
            detachElement(element);
        }
        for (const std::shared_ptr<OnChangeListener>& listener : *onChangeListeners) {
            listener->onElementsRemoved();
        }
    }
    
    void VectorDataSource::attachElement(const std::shared_ptr<VectorElement>& element) {
        element->attachToDataSource(shared_from_this());
    }
    
    void VectorDataSource::detachElement(const std::shared_ptr<VectorElement>& element) {
        element->detachFromDataSource();
    }
    
}
