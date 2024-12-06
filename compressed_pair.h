#pragma once

#include <exception>
#include <memory>

class ControlBlockBase {
public:
    size_t strong_counter_;
    size_t weak_counter_;
    virtual void DeleteData() = 0;
    virtual ~ControlBlockBase() = default;
};

template <typename T>
class ControlBlockWithObject : public ControlBlockBase {
public:
    alignas(T) unsigned char buf_[sizeof(T)];

    template <typename... Args>
    ControlBlockWithObject(Args&&... args) {
        strong_counter_ = 1;
        weak_counter_ = 0;
        ptr_ = new (&buf_) T(std::forward<Args>(args)...);
    }

    T* ptr_;

    void DeleteData() override {
        reinterpret_cast<T*>(&buf_)->~T();
    }
};

template <typename T>
class ControlBlockWithPointer : public ControlBlockBase {
public:
    ControlBlockWithPointer(T* ptr) : ptr_(ptr) {
        strong_counter_ = 1;
        weak_counter_ = 0;
    }

    T* ptr_;

    void DeleteData() override {
        delete ptr_;
    }
};

struct NeedNewObject {};

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
