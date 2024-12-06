#include "intrusive.h"
#include "shared.h"
#include "weak.h"

#include <cassert>

void SharedEmptyState() {
    SharedPtr<int> a, b;

    b = a;
    SharedPtr c(a);
    b = std::move(c);

    assert(a.Get() == nullptr);
    assert(b.Get() == nullptr);
    assert(c.Get() == nullptr);
}

void SharedCopyMove() {
    SharedPtr<std::string> a(new std::string("aba"));
    std::string* ptr;
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



int main() {

}
