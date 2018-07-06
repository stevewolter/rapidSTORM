#ifndef VIRTUAL_CLONE_ALLOCATOR_HPP
#define VIRTUAL_CLONE_ALLOCATOR_HPP

template <typename BaseClass>
class VirtualCloneAllocator {
  public:
    static BaseClass* allocate_clone(const BaseClass& a) { return a.clone(); }
    static void deallocate_clone(const BaseClass* a) { delete a; }
};

#endif
