/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_VECTORDATASOURCE_H_
#define _CARTO_VECTORDATASOURCE_H_

#include "datasources/components/VectorData.h"

#include <memory>
#include <vector>

namespace carto {
    class ViewState;
    class CullState;
    class Projection;
    
    /**
     * Abstract base class for envelope based vector data sources. It provides default implementation
     * for listener registration and other common data source methods.
     * Subclasses need to define their own implementations of loadElements method.
     *
     * The draw order of vector elements within the data source is undefined.
     */
    class VectorDataSource : public std::enable_shared_from_this<VectorDataSource> {
    public:
        /**
         * Interface for monitoring data source change events.
         */
        struct OnChangeListener {
            virtual ~OnChangeListener() { }
    
            /**
             * Listener method that gets called when a vector element was added to the data source.
             * @param element The added vector element.
             */
            virtual void onElementAdded(const std::shared_ptr<VectorElement>& element) = 0;
            /**
             * Listener method that gets called when a vector element attached to the data source
             * has changed and needs to be updated.
             * @param element The changed vector element.
             */
            virtual void onElementChanged(const std::shared_ptr<VectorElement>& element) = 0;
            /**
             * Listener method that gets called before a vector element is removed from the data source.
             * @param element The removed vector element.
             */
            virtual void onElementRemoved(const std::shared_ptr<VectorElement>& element) = 0;
            /**
             * Listener method that gets called when multiple vector elements were added to the data source.
             * @param elements The vector of added vector elements.
             */
            virtual void onElementsAdded(const std::vector<std::shared_ptr<VectorElement> >& elements) = 0;
            /**
             * Listener method that gets called when all existing vector elements attached to the data source
             * have changed and need to be updated.
             */
            virtual void onElementsChanged() = 0;
            /**
             * Listener method that gets called before all vector elements are removed from the data source.
             */
            virtual void onElementsRemoved() = 0;
        };
    
        virtual ~VectorDataSource();
        
        /**
         * Returns the projection used by this data source.
         * @return The projection used by this data source.
         */
        std::shared_ptr<Projection> getProjection() const;
    
        /**
         * Loads all the elements within the defined envelope.
         * @param cullState State for describing view parameters and conservative view envelope.
         * @return The vector of loaded vector elements. If no elements are available, null may be returned.
         */
        virtual std::shared_ptr<VectorData> loadElements(const std::shared_ptr<CullState>& cullState) = 0;
        
        /**
         * Notifies listeners that all vector elements have changed. This method refreshes all the existing 
         * vector elements in the data source.
         */
        void notifyElementsChanged();
    
        /**
         * Registers listener for data source change events.
         * @param listener The listener for change events.
         */
        void registerOnChangeListener(const std::shared_ptr<OnChangeListener>& listener);
        /**
         * Unregisters listener from data source change events.
         * @param listener The previously added listener.
         */
        void unregisterOnChangeListener(const std::shared_ptr<OnChangeListener>& listener);
    
    protected:
        friend class VectorElement;
    
        /**
         * Constructs an abstract VectorDataSource object.
         * @param projection The projection used by this data source.
         */
        explicit VectorDataSource(const std::shared_ptr<Projection>& projection);

        float calculateGeometrySimplifierScale(const ViewState& viewState) const;
        
        virtual void notifyElementAdded(const std::shared_ptr<VectorElement>& element);
        virtual void notifyElementChanged(const std::shared_ptr<VectorElement>& element);
        virtual void notifyElementRemoved(const std::shared_ptr<VectorElement>& element);
        virtual void notifyElementsAdded(const std::vector<std::shared_ptr<VectorElement> >& elements);
        virtual void notifyElementsRemoved(const std::vector<std::shared_ptr<VectorElement> >& elements);

        virtual void attachElement(const std::shared_ptr<VectorElement>& element);
        virtual void detachElement(const std::shared_ptr<VectorElement>& element);

        std::shared_ptr<Projection> _projection;
    
    private:
        std::shared_ptr<std::vector<std::shared_ptr<OnChangeListener> > > _onChangeListeners;
        mutable std::mutex _onChangeListenersMutex;
    };
    
}

#endif
