#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "weak.h"
#include <cstddef>  // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
class EnableBase {};

template <typename T>
class EnableSharedFromThis : public EnableBase {
public:
    EnableSharedFromThis() noexcept {
    }

    SharedPtr<T> SharedFromThis() {
        return weak_this_.Lock();
    }
    SharedPtr<const T> SharedFromThis() const {
        return weak_this_.Lock();
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this_;
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return weak_this_;
    }
    WeakPtr<T> weak_this_;
};

template <typename T>
class SharedPtr {
    template <typename Y>
    friend class WeakPtr;
    template <typename Y>
    friend class SharedPtr;

public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() {
        cb_ = nullptr;
        ptr_ = nullptr;
    }
    SharedPtr(std::nullptr_t) {
        cb_ = nullptr;
        ptr_ = nullptr;
    }
    template <typename... Args>
    SharedPtr(NeedNewObject, Args&&... args) {
        cb_ = new ControlBlockWithObject<T>(std::forward<Args>(args)...);
        ptr_ = (dynamic_cast<ControlBlockWithObject<T>*>(cb_)->ptr_);
        if constexpr (std::is_convertible_v<T, EnableBase>) {
            (*ptr_).weak_this_ = std::move(WeakPtr<T>(*this));
        }
    }

    explicit SharedPtr(T* ptr) : ptr_(ptr), cb_(new ControlBlockWithPointer<T>(ptr)) {
        if constexpr (std::is_convertible_v<T, EnableBase>) {
            (*ptr_).weak_this_ = std::move(WeakPtr<T>(*this));
        }
    }
    template <typename Y>
    explicit SharedPtr(Y* ptr) : ptr_(ptr), cb_(new ControlBlockWithPointer<Y>(ptr)) {
        if constexpr (std::is_convertible_v<Y, EnableBase>) {
            (*ptr_).weak_this_ = std::move(WeakPtr<T>(*this));
        }
    }

    SharedPtr(const SharedPtr& other) {
        ptr_ = other.ptr_;
        cb_ = other.cb_;
        if (cb_ != nullptr) {
            cb_->strong_counter_++;
        }
    }
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) {
        static_assert(std::is_convertible_v<Y*, T*>, "Inconvertible types");
        ptr_ = other.ptr_;
        cb_ = other.cb_;
        if (cb_ != nullptr) {
            cb_->strong_counter_++;
        }
    }
    SharedPtr(SharedPtr&& other) {
        ptr_ = std::move(other.ptr_);
        cb_ = std::move(other.cb_);
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }
    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) {
        static_assert(std::is_convertible_v<Y*, T*>, "Inconvertible types");
        ptr_ = std::move(other.ptr_);
        cb_ = std::move(other.cb_);
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        ptr_ = ptr;
        cb_ = other.cb_;
        if (cb_ != nullptr) {
            cb_->strong_counter_++;
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr{};
        }
        ptr_ = other.ptr_;
        cb_ = other.cb_;
        if (cb_ != nullptr) {
            cb_->strong_counter_++;
        }
    }

    template <typename Y>
    SharedPtr(const WeakPtr<Y>& other) {
        if (other.Expired()) {
            throw BadWeakPtr{};
        }
        ptr_ = other.ptr_;
        cb_ = other.cb_;
        if (cb_ != nullptr) {
            cb_->strong_counter_++;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        Reset();
        ptr_ = std::move(other.ptr_);
        cb_ = std::move(other.cb_);
        if (cb_ != nullptr) {
            cb_->strong_counter_++;
        }
        return *this;
    }
    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        static_assert(std::is_convertible_v<Y*, T*>, "Inconvertible types");
        Reset();
        ptr_ = other.ptr_;
        cb_ = other.cb_;
        if (cb_ != nullptr) {
            cb_->strong_counter_++;
        }
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) {
        Reset();
        ptr_ = std::move(other.ptr_);
        cb_ = std::move(other.cb_);
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
        return *this;
    }
    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        static_assert(std::is_convertible_v<Y*, T*>, "Inconvertible types");
        Reset();
        ptr_ = std::move(other.ptr_);
        cb_ = std::move(other.cb_);
        other.ptr_ = nullptr;
        other.cb_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (cb_ != nullptr) {
            cb_->strong_counter_--;
            if (cb_->strong_counter_ == 0) {
                cb_->strong_counter_++;
                if constexpr (std::is_convertible_v<T, EnableBase>) {
                    (*ptr_).weak_this_.Reset();
                }
                cb_->strong_counter_--;
                cb_->DeleteData();
            }
            if (cb_->strong_counter_ == 0 && cb_->weak_counter_ == 0) {
                delete cb_;
            }
        }
        ptr_ = nullptr;
        cb_ = nullptr;
    }
    template <typename Y>
    void Reset(Y* ptr) {
        if (cb_ != nullptr) {
            cb_->strong_counter_--;
            if (cb_->strong_counter_ == 0) {
                cb_->strong_counter_++;
                if constexpr (std::is_convertible_v<T, EnableBase>) {
                    (*ptr_).weak_this_.Reset();
                }
                cb_->strong_counter_--;
                cb_->DeleteData();
            }
            if (cb_->strong_counter_ == 0 && cb_->weak_counter_ == 0) {
                delete cb_;
            }
        }
        ptr_ = ptr;
        cb_ = new ControlBlockWithPointer<Y>(ptr);
    }
    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(cb_, other.cb_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return Get();
    }
    size_t UseCount() const {
        if (cb_ == nullptr) {
            return 0;
        }
        return cb_->strong_counter_;
    }
    explicit operator bool() const {
        return Get() != nullptr;
    }

private:
    T* ptr_;
    ControlBlockBase* cb_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(NeedNewObject{}, std::forward<Args>(args)...);
}
