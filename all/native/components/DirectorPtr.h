/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_DIRECTORPTR_H_
#define _CARTO_DIRECTORPTR_H_

#include "Director.h"

#include <memory>

namespace carto {
    
    template <typename T>
    class DirectorPtr {
    public:
        DirectorPtr() : _ptr() { }

        DirectorPtr(const DirectorPtr<T>& directorPtr) : _ptr(directorPtr._ptr) {
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->retainDirector();
            }
        }
        
        explicit DirectorPtr(const std::shared_ptr<T>& ptr) : _ptr(ptr) {
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->retainDirector();
            }
        }
        
        ~DirectorPtr() {
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->releaseDirector();
            }
        }
        
        const std::shared_ptr<T>& get() const {
            return _ptr;
        }
        
        T* operator -> () const {
            return _ptr.get();
        }
        
        T& operator * () const {
            return *_ptr;
        }
        
        operator bool() const {
            return (bool)_ptr;
        }
        
        bool operator == (const DirectorPtr<T>& other) const {
            return _ptr == other._ptr;
        }

        bool operator != (const DirectorPtr<T>& other) const {
            return _ptr != other._ptr;
        }
        
        DirectorPtr<T>& operator = (DirectorPtr<T> directorPtr) {
            std::swap(_ptr, directorPtr._ptr);
            return *this;
        }
        
    private:
        std::shared_ptr<T> _ptr;
    };

}

namespace std {

    template <typename T>
    struct hash<carto::DirectorPtr<T> > {
        size_t operator() (const carto::DirectorPtr<T>& ptr) const {
            return hash<shared_ptr<T> >()(ptr.get());
        }
    };

}

#endif
