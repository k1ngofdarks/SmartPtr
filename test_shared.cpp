#include "intrusive.h"
#include "shared.h"
#include "weak.h"

#include <cassert>

///================================================================================================///

void SharedEmptyState() {
    SharedPtr<int> a, b;

    b = a;
    SharedPtr c(a);
    b = std::move(c);

    assert(a.Get() == nullptr);
    assert(b.Get() == nullptr);
    assert(c.Get() == nullptr);
}

///================================================================================================///

void SharedCopyMove() {
    SharedPtr<std::string> a(new std::string("aba"));
    std::string *ptr;
    {
        SharedPtr b(a);
        SharedPtr c(a);
        ptr = c.Get();
    }
    assert(ptr == a.Get());
    assert(*ptr == "aba");

    SharedPtr<std::string> b(new std::string("caba"));
    {
        SharedPtr c(b);
        SharedPtr d(b);
        d = std::move(a);
        assert(*c == "caba");
        assert(*d == "aba");
        b.Reset(new std::string("test"));
        assert(*c == "caba");
    }
    assert(*b == "test");

    SharedPtr<std::string> end;
    {
        SharedPtr<std::string> d(new std::string("delete"));
        d = b;
        SharedPtr c(std::move(b));
        assert(*d == "test");
        assert(*c == "test");
        d = d;  // NOLINT
        c = end;
        d.Reset(new std::string("delete"));
        end = d;
    }

    assert(*end == "delete");
}

///================================================================================================///


struct ModifiersB {
    static int count;

    ModifiersB() {
        ++count;
    }

    ModifiersB(const ModifiersB &) {
        ++count;
    }

    virtual ~ModifiersB() {
        --count;
    }
};

int ModifiersB::count = 0;

struct ModifiersA : public ModifiersB {
    static int count;

    ModifiersA() {
        ++count;
    }

    ModifiersA(const ModifiersA &other) : ModifiersB(other) {
        ++count;
    }

    ~ModifiersA() {
        --count;
    }
};

int ModifiersA::count = 0;

struct ModifiersC {
    static int count;

    ModifiersC() {
        ++count;
    }

    ModifiersC(const ModifiersC &) {
        ++count;
    }

    ~ModifiersC() {
        --count;
    }
};

int ModifiersC::count = 0;

void SharedModifiers() {
    {   // SECTION("Reset") 
        {
            SharedPtr<ModifiersB> p(new ModifiersB);
            p.Reset();
            (ModifiersA::count == 0);
            assert(ModifiersB::count == 0);
            assert(p.UseCount() == 0);
            assert(p.Get() == nullptr);
        }
        assert(ModifiersA::count == 0);
        {
            SharedPtr<ModifiersB> p;
            p.Reset();
            assert(ModifiersA::count == 0);
            assert(ModifiersB::count == 0);
            assert(p.UseCount() == 0);
            assert(p.Get() == nullptr);
        }
        assert(ModifiersA::count == 0);
    }

    {   // SECTION("Reset ptr")
        {
            SharedPtr<ModifiersB> p(new ModifiersB);
            ModifiersA *ptr = new ModifiersA;
            p.Reset(ptr);
            assert(ModifiersA::count == 1);
            assert(ModifiersB::count == 1);
            assert(p.UseCount() == 1);
            assert(p.Get() == ptr);
        }
        assert(ModifiersA::count == 0);
        {
            SharedPtr<ModifiersB> p;
            ModifiersA *ptr = new ModifiersA;
            p.Reset(ptr);
            assert(ModifiersA::count == 1);
            assert(ModifiersB::count == 1);
            assert(p.UseCount() == 1);
            assert(p.Get() == ptr);
        }
        assert(ModifiersA::count == 0);
    }

    {   // SECTION("Swap") {
        {
            ModifiersC *ptr1 = new ModifiersC;
            ModifiersC *ptr2 = new ModifiersC;
            SharedPtr<ModifiersC> p1(ptr1);
            {
                SharedPtr<ModifiersC> p2(ptr2);
                p1.Swap(p2);
                assert(p1.UseCount() == 1);
                assert(p1.Get() == ptr2);
                assert(p2.UseCount() == 1);
                assert(p2.Get() == ptr1);
                assert(ModifiersC::count == 2);
            }
            assert(p1.UseCount() == 1);
            assert(p1.Get() == ptr2);
            assert(ModifiersC::count == 1);
        }
        assert(ModifiersC::count == 0);
        {
            ModifiersC *ptr1 = new ModifiersC;
            ModifiersC *ptr2 = nullptr;
            SharedPtr<ModifiersC> p1(ptr1);
            {
                SharedPtr<ModifiersC> p2;
                p1.Swap(p2);
                assert(p1.UseCount() == 0);
                assert(p1.Get() == ptr2);
                assert(p2.UseCount() == 1);
                assert(p2.Get() == ptr1);
                assert(ModifiersC::count == 1);
            }
            assert(p1.UseCount() == 0);
            assert(p1.Get() == ptr2);
            assert(ModifiersC::count == 0);
        }
        assert(ModifiersC::count == 0);
        {
            ModifiersC *ptr1 = nullptr;
            ModifiersC *ptr2 = new ModifiersC;
            SharedPtr<ModifiersC> p1;
            {
                SharedPtr<ModifiersC> p2(ptr2);
                p1.Swap(p2);
                assert(p1.UseCount() == 1);
                assert(p1.Get() == ptr2);
                assert(p2.UseCount() == 0);
                assert(p2.Get() == ptr1);
                assert(ModifiersC::count == 1);
            }
            assert(p1.UseCount() == 1);
            assert(p1.Get() == ptr2);
            assert(ModifiersC::count == 1);
        }
        assert(ModifiersC::count == 0);
        {
            ModifiersC *ptr1 = nullptr;
            ModifiersC *ptr2 = nullptr;
            SharedPtr<ModifiersC> p1;
            {
                SharedPtr<ModifiersC> p2;
                p1.Swap(p2);
                assert(p1.UseCount() == 0);
                assert(p1.Get() == ptr2);
                assert(p2.UseCount() == 0);
                assert(p2.Get() == ptr1);
                assert(ModifiersC::count == 0);
            }
            assert(p1.UseCount() == 0);
            assert(p1.Get() == ptr2);
            assert(ModifiersC::count == 0);
        }
        assert(ModifiersC::count == 0);
    }
}

///================================================================================================///

struct OperatorBoolA {
    int a;

    virtual ~OperatorBoolA() {};
};

void SharedObservers() {
    {   // SECTION("operator->") 
        const SharedPtr<std::pair<int, int>> p(new std::pair<int, int>(3, 4));
        assert(p->first == 3);
        assert(p->second == 4);
        p->first = 5;
        p->second = 6;
        assert(p->first == 5);
        assert(p->second == 6);
    }

    {   //  SECTION("Dereference") 
        const SharedPtr<int> p(new int(32));
        assert(*p == 32);
        *p = 3;
        assert(*p == 3);
    }

    {   // SECTION("operator bool")
        static_assert(std::is_constructible<bool, SharedPtr<OperatorBoolA>>::value, "");
        static_assert(!std::is_convertible<SharedPtr<OperatorBoolA>, bool>::value, "");
        {
            const SharedPtr<int> p(new int(32));
            assert(p);
        }
        {
            const SharedPtr<int> p;
            assert(!p);
        }
    }
}

///================================================================================================///

struct Pinned {
    Pinned(int tag) : tag_(tag) {
    }

    Pinned(const Pinned &a) = delete;

    Pinned(Pinned &&a) = delete;

    Pinned &operator=(const Pinned &a) = delete;

    Pinned &operator=(Pinned &&a) = delete;

    ~Pinned() = default;

    int GetTag() const {
        return tag_;
    }

private:
    int tag_;
};

void SharedNoCopy() {
    SharedPtr<Pinned> p(new Pinned(1));
}

///================================================================================================///

struct D {
    D(Pinned &pinned, std::unique_ptr<int> &&p)
            : some_uncopyable_thing_(std::move(p)), pinned_(pinned) {
    }

    int GetUP() const {
        return *some_uncopyable_thing_;
    }

    Pinned &GetPinned() const {
        return pinned_;
    }

private:
    std::unique_ptr<int> some_uncopyable_thing_;
    Pinned &pinned_;
};

struct Throwing {
    Throwing() {
        throw 42;
    }
};

void SharedMake() {
    {    //  SECTION("Parameters passing")
        auto p_int = std::make_unique<int>(42);
        Pinned pinned(1312);
        auto p = MakeShared<D>(pinned, std::move(p_int));

        assert(p->GetUP() == 42);
        assert(p->GetPinned().GetTag() == 1312);
    }

    {   // SECTION("Constructed only once")
        auto sp = MakeShared<Pinned>(1);
    }

    {   // SECTION("Faulty constructor")
        try {
            auto sp = MakeShared<Throwing>();
        } catch (...) {
        }
    }
}

///================================================================================================///

struct Data {
    static bool data_was_deleted;

    int x;
    double y;

    ~Data() {
        data_was_deleted = true;
    }
};

bool Data::data_was_deleted = false;

void SharedAliasing() {
    {   // SECTION("It just exists") 
        SharedPtr<Data> sp(new Data{42, 3.14});

        SharedPtr<double> sp2(sp, &sp->y);

        assert(*sp2 == 3.14);
    }

    {   // SECTION("Lifetime extension")
        {
            Data::data_was_deleted = false;
            SharedPtr<double> sp3;
            {
                SharedPtr<Data> sp(new Data{42, 3.14});
                SharedPtr<double> sp2(sp, &sp->y);
                sp3 = sp2;
            }
            assert(*sp3 == 3.14);
            assert(!Data::data_was_deleted);
        }
        assert(Data::data_was_deleted);
    }
}

///================================================================================================///

class Base {
public:
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    static bool i_was_deleted;

    ~Derived() {
        i_was_deleted = true;
    }
};

bool Derived::i_was_deleted = false;

void SharedTypeConversions() {
    {   // SECTION("Destruction") 
        Derived::i_was_deleted = false;
        { SharedPtr<Base> sb(new Derived); }
        assert(Derived::i_was_deleted);
    }

    {   // SECTION("Constness") 
        SharedPtr<int> s1(new int(42));
        SharedPtr<const int> s2 = s1;

        SharedPtr<const int> s3 = std::move(s1);
        assert(!s1);
        assert(s2.UseCount() == 2);

        s1.Reset(new int(43));
        s2 = s1;
        s3 = std::move(s1);
        assert(!s1);
        assert(s3.UseCount() == 2);
    }
}

///================================================================================================///

struct A {
    ~A() = default;
};

struct B : A {
    ~B() {
        destructor_called = true;
    }

    static bool destructor_called;
};

bool B::destructor_called = false;


void SharedDestructor() {
    {   // SECTION("Regular constructor") 
        B::destructor_called = false;
        { SharedPtr<A>(new B()); }
        assert(B::destructor_called);
    }

    {   // SECTION("MakeShared") 
        B::destructor_called = false;
        { SharedPtr<A> ptr = MakeShared<B>(); }
        assert(B::destructor_called);
    }

    {   // SECTION("Reset") 
        B::destructor_called = false;
        {
            SharedPtr<A> ptr(new A);
            ptr.Reset(new B);
        }
        assert(B::destructor_called);
    }
}

///================================================================================================///

int main() {
    SharedEmptyState();
    SharedCopyMove();
    SharedModifiers();
    SharedObservers();
    SharedNoCopy();
    SharedMake();
    SharedAliasing();
    SharedTypeConversions();
    SharedDestructor();
    return 0;
}
