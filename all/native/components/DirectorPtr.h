/*
 * Copyright (c) 2016 CartoDB. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#ifndef _CARTO_DIRECTORPTR_H_
#define _CARTO_DIRECTORPTR_H_

#include "components/Director.h"

#include <memory>
#include <mutex>

namespace carto {

    template <typename T>
    class DirectorPtr;
    
    template <typename T>
    class ThreadSafeDirectorPtr {
    public:
        ThreadSafeDirectorPtr() : _ptr(), _mutex() { }
        ThreadSafeDirectorPtr(const std::shared_ptr<T>& ptr) : _ptr(ptr), _mutex() {
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->retainDirector();
            }
        }
        ThreadSafeDirectorPtr(const ThreadSafeDirectorPtr& other) : _ptr(), _mutex() {
            std::lock_guard<std::mutex> lock(other._mutex);
            _ptr = other._ptr;
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->retainDirector();
            }
        }

        ~ThreadSafeDirectorPtr() {
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->releaseDirector();
            }
        }
        
        std::shared_ptr<T> get() const {
            std::lock_guard<std::mutex> lock(_mutex);
            return _ptr;
        }

        void set(const std::shared_ptr<T>& ptr) {
            std::lock_guard<std::mutex> lock(_mutex);
            if (auto director = std::dynamic_pointer_cast<Director>(ptr)) {
                director->retainDirector();
            }
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->releaseDirector();
            }
            _ptr = ptr;
        }
        
        bool operator == (const ThreadSafeDirectorPtr& other) const {
            std::lock(_mutex, other._mutex);
            std::lock_guard<std::mutex> lock1(_mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lock2(other._mutex, std::adopt_lock);
            return _ptr == other._ptr;
        }

        bool operator != (const ThreadSafeDirectorPtr& other) const {
            return !(*this == other);
        }
        
        ThreadSafeDirectorPtr& operator = (ThreadSafeDirectorPtr other) {
            std::lock(_mutex, other._mutex);
            std::lock_guard<std::mutex> lock1(_mutex, std::adopt_lock);
            std::lock_guard<std::mutex> lock2(other._mutex, std::adopt_lock);
            std::swap(_ptr, other._ptr);
            return *this;
        }

    private:
        friend DirectorPtr<T>;

        std::shared_ptr<T> _ptr;
        mutable std::mutex _mutex;
    };

    template <typename T>
    class DirectorPtr {
    public:
        DirectorPtr() : _ptr() { }
        DirectorPtr(const std::shared_ptr<T>& ptr) : _ptr(ptr) {
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->retainDirector();
            }
        }
        DirectorPtr(const DirectorPtr& other) : _ptr(other._ptr) {
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->retainDirector();
            }
        }
        DirectorPtr(const ThreadSafeDirectorPtr<T>& other) : _ptr() {
            std::lock_guard<std::mutex> lock(other._mutex);
            _ptr = other._ptr;
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->retainDirector();
            }
        }
        
        ~DirectorPtr() {
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->releaseDirector();
            }
        }
        
        std::shared_ptr<T> get() const {
            return _ptr;
        }

        void set(const std::shared_ptr<T>& ptr) {
            if (auto director = std::dynamic_pointer_cast<Director>(ptr)) {
                director->retainDirector();
            }
            if (auto director = std::dynamic_pointer_cast<Director>(_ptr)) {
                director->releaseDirector();
            }
            _ptr = ptr;
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
        
        bool operator == (const DirectorPtr& other) const {
            return _ptr == other._ptr;
        }

        bool operator != (const DirectorPtr& other) const {
            return !(*this == other);
        }
        
        DirectorPtr& operator = (DirectorPtr other) {
            std::swap(_ptr, other._ptr);
            return *this;
        }

        DirectorPtr& operator = (const ThreadSafeDirectorPtr<T>& other) {
            std::lock_guard<std::mutex> lock(other._mutex);
            set(other._ptr);
            return *this;
        }
        
    private:
        std::shared_ptr<T> _ptr;
    };

}

namespace std {

    template <typename T>
    struct hash<carto::DirectorPtr<T> > {
        size_t operator() (const carto::DirectorPtr<T>& directorPtr) const {
            return hash<shared_ptr<T> >()(directorPtr.get());
        }
    };

}

#endif
