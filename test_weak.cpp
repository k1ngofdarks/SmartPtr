#include "intrusive.h"
#include "shared.h"
#include "weak.h"

#include <cassert>

///================================================================================================///

void WeakEmpty() {
    WeakPtr<int> a;
    WeakPtr<int> b;
    a = b;
    WeakPtr c(a);
    b = std::move(c);

    auto shared = b.Lock();
    assert(shared.Get() == nullptr);
}

///================================================================================================///

void WeakPtrCopyMove() {
    SharedPtr<std::string> a(new std::string("aba"));
    WeakPtr<std::string> b(a);
    WeakPtr<std::string> empty;
    WeakPtr c(b);
    WeakPtr<std::string> d(a);

    assert(d.UseCount() == 1);

    assert(!c.Expired());
    c = empty;
    assert(c.Expired());

    b = std::move(c);

    WeakPtr e(std::move(d));
    assert(d.Lock().Get() == nullptr);

    auto locked = e.Lock();
    assert(*locked == "aba");

    WeakPtr<std::string> start(a);
    {
        SharedPtr a2(a);
        WeakPtr<std::string> f(a2);
        auto cur_lock = f.Lock();
        assert(cur_lock.Get() == SharedPtr(start).Get());
    }
}

///================================================================================================///

void WeakModifiers() {
    {   // SECTION("Reset") 
        {
            SharedPtr<int> shared = MakeShared<int>(42), shared2 = shared, shared3 = shared2;
            WeakPtr<int> weak = WeakPtr<int>{shared};
            assert(shared.UseCount() == 3);
            assert(weak.UseCount() == 3);
            assert(!weak.Expired());
            weak.Reset();
            assert(shared.UseCount() == 3);
            assert(weak.UseCount() == 0);
            assert(weak.Expired());
        }
    }

    {   // SECTION("Reset deletes block") 
        WeakPtr<int>* wp;
        {
            auto sp = MakeShared<int>();
            wp = new WeakPtr<int>(sp);
        }
        wp->Reset();
        delete wp;
    }

    {   // SECTION("Swap") 
        {
            SharedPtr<int> shared = MakeShared<int>(42), shared3 = shared;
            SharedPtr<int> shared2 = MakeShared<int>(13);
            WeakPtr<int> weak = WeakPtr<int>{shared};
            WeakPtr<int> weak2 = WeakPtr<int>{shared2};
            assert(weak.UseCount() == 2);
            assert(weak2.UseCount() == 1);
            weak.Swap(weak2);
            assert(weak.UseCount() == 1);
            assert(weak2.UseCount() == 2);
        }
    }
}

///================================================================================================///

void WeakExpiration() {
    WeakPtr<std::string>* a;
    {
        SharedPtr<std::string> b(new std::string("aba"));
        SharedPtr c(b);
        a = new WeakPtr<std::string>(c);
        auto test = a->Lock();
        assert(*test == "aba");
        assert(!a->Expired());
    }
    assert(a->Expired());
    delete a;
}

///================================================================================================///

void WeakExtendsShared() {
    SharedPtr<std::string>* b = new SharedPtr<std::string>(new std::string("aba"));
    WeakPtr<std::string> c(*b);
    auto a = c.Lock();
    delete b;
    assert(!c.Expired());
    assert(*a == "aba");
}

///================================================================================================///

void SharedFromWeak() {
    SharedPtr<std::string>* x = new SharedPtr<std::string>(new std::string("aba"));
    WeakPtr<std::string> y(*x);
    delete x;
    assert(y.Expired());
    SharedPtr z = y.Lock();
    assert(z.Get() == nullptr);
}

///================================================================================================///

int main() {
    WeakEmpty();
    WeakPtrCopyMove();
    WeakModifiers();
    WeakExpiration();
    WeakExtendsShared();
    SharedFromWeak();

    return 0;
}