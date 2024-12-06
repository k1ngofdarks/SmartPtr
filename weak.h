#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
    template <typename Y>
    friend class WeakPtr;
    template <typename Y>
    friend class SharedPtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
        cb_ = nullptr;
        ptr_ = nullptr;
    }

    WeakPtr(const WeakPtr& other) {
        cb_ = other.cb_;
        ptr_ = other.ptr_;
        if (cb_ != nullptr) {
            cb_->weak_counter_++;
        }
    }

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other) {
        static_assert(std::is_convertible_v<Y*, T*>, "Inconvertible types");
        cb_ = other.cb_;
        ptr_ = other.ptr_;
        if (cb_ != nullptr) {
            cb_->weak_counter_++;
        }
    }

    WeakPtr(WeakPtr&& other) {
        cb_ = std::move(other.cb_);
        ptr_ = std::move(other.ptr_);
        other.cb_ = nullptr;
        other.ptr_ = nullptr;
    }

    template <typename Y>
    WeakPtr(WeakPtr<Y>&& other) {
        static_assert(std::is_convertible_v<Y*, T*>, "Inconvertible types");
        cb_ = std::move(other.cb_);
        ptr_ = std::move(other.ptr_);
        other.cb_ = nullptr;
        other.ptr_ = nullptr;
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        cb_ = other.cb_;
        ptr_ = other.ptr_;
        if (cb_ != nullptr) {
            cb_->weak_counter_++;
        }
    }

    template <typename Y>
    WeakPtr(const SharedPtr<Y>& other) {
        static_assert(std::is_convertible_v<Y*, T*>, "Inconvertible types");
        cb_ = other.cb_;
        ptr_ = other.ptr_;
        if (cb_ != nullptr) {
            cb_->weak_counter_++;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        Reset();
        cb_ = other.cb_;
        ptr_ = other.ptr_;
        if (cb_ != nullptr) {
            cb_->weak_counter_++;
        }
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(const WeakPtr<Y>& other) {
        static_assert(std::is_convertible_v<Y*, T*>, "Inconvertible types");
        Reset();
        cb_ = other.cb_;
        ptr_ = other.ptr_;
        if (cb_ != nullptr) {
            cb_->weak_counter_++;
        }
        return *this;
    }

    WeakPtr& operator=(WeakPtr&& other) {
        Reset();
        cb_ = std::move(other.cb_);
        ptr_ = std::move(other.ptr_);
        other.cb_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    template <typename Y>
    WeakPtr& operator=(WeakPtr<Y>&& other) {
        static_assert(std::is_convertible_v<Y*, T*>, "Inconvertible types");
        Reset();
        cb_ = std::move(other.cb_);
        ptr_ = std::move(other.ptr_);
        other.cb_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (cb_ != nullptr) {
            cb_->weak_counter_--;
            if (cb_->strong_counter_ == 0 && cb_->weak_counter_ == 0) {
                delete cb_;
            }
        }
        ptr_ = nullptr;
        cb_ = nullptr;
    }

    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(cb_, other.cb_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }

    size_t UseCount() const {
        if (cb_ == nullptr) {
            return 0;
        }
        return cb_->strong_counter_;
    }
    bool Expired() const {
        return UseCount() == 0;
    }
    SharedPtr<T> Lock() const {
        if (Expired()) {
            return SharedPtr<T>(nullptr);
        }
        return SharedPtr<T>(*this);
    }

private:
    T* ptr_;
    ControlBlockBase* cb_;
};
