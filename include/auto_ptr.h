//
// Here is a sample implementation of Version 2 auto_ptr
// as an excerpt from a  March 1996 newsgroup posting by Greg Colvin
//
template<class X>
class auto_ptr {
    mutable bool owner;
    X* px;
    template<class Y> friend class auto_ptr;

public:
    explicit auto_ptr(X* p=0): owner(p), px(p) {}
    template<class Y>
    auto_ptr(const auto_ptr<Y>& r): owner(r.owner), px(r.release()) {}
    template<class Y>
    auto_ptr& operator=(const auto_ptr<Y>& r) {
        if ((void*)&r != (void*)this) {
            if (owner)
                delete px;
            owner = r.owner;
            px = r.release();
        }
        return *this;
    }
    ~auto_ptr() { if (owner) delete px; }
    X& operator*() const { return *px; }
    X* operator->() const { return px; }
    X* get() const { return px; }
    X* release() const { owner = 0; return px; }
};
