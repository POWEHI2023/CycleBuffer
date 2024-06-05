#include "cbuffer.h"

template <typename T>
c_buffer<T>::~c_buffer() noexcept {
    _ElemType *_ptr = nullptr;
    while (!empty()) {
        _ptr = &_mem[_head];
        _head = (_head + 1) % _capacity;

        if (_ptr != nullptr) 
            _ptr->~_ElemType();
    }
}

template <typename T>
typename c_buffer<T>::_ElemType c_buffer<T>::front() const {
    if (empty()) 
        throw PopEmptyBufferError("can not get front element because current buffer is empty.");

    // copy construct a new element
    _ElemType _ret(_mem[_head]);
    return _ret;
}

template <typename T>
size_t c_buffer<T>::size() const noexcept { return _size; }
template <typename T>
size_t c_buffer<T>::capacity() const noexcept { return _capacity - 1; }
template <typename T>
bool c_buffer<T>::empty() const noexcept { return _head == _tail; }
template <typename T>
bool c_buffer<T>::full() const noexcept { return ((_tail + 1) % _capacity) == _head; }
template <typename T>
bool c_buffer<T>::insert(const _ElemType& _elem) noexcept { return __proxy_insert<decltype(_elem), false>(_elem); }
template <typename T>
void c_buffer<T>::pop() noexcept { __pop(); }
template <typename T>
bool c_buffer<T>::atomic_insert(const _ElemType& _elem) noexcept { return __proxy_insert<decltype(_elem), true>(_elem); }
template <typename T>
const typename c_buffer<T>::_ElemType c_buffer<T>::atomic_pop() { return __atomic_pop(); }

// 
// private function implication
//

template <typename T>
bool c_buffer<T>::__insert(const _ElemType& _elem) noexcept {
    if (full()) return false;

    _ElemType* _ptr = &_mem[_tail];
    try {
        new(_ptr) _ElemType(_elem);
    } catch (std::exception e) {
        printf("ERROR: insert failed (%s)\n", e.what());
        return false;
    }
    _tail = (_tail + 1) % _capacity;
    return true;
}

template <typename T>
void c_buffer<T>::__pop() noexcept {
    _ElemType *_ptr = &_mem[_head];
    _ptr->~_ElemType();

    _head = (_head + 1) % _capacity;
    _size--;
}

template <typename T>
bool c_buffer<T>::__atomic_insert(const _ElemType& _elem) noexcept { return false; }
template <typename T>
const typename c_buffer<T>::_ElemType c_buffer<T>::__atomic_pop() { return false; }



// 
// copy and mvoe constructor
// 

template <typename T>
c_buffer<T>::c_buffer(const c_buffer<T>& buffer) {
    _size = buffer._size;
    _capacity = buffer._capacity;
    _head = buffer._head;
    _tail = buffer._tail;
    
    size_t _total_size = sizeof(_ElemType) * (_size + 1);
    _mem = (_ElemType*)(malloc(_total_size));

    memcpy(_mem, buffer._mem, _total_size);
}

template <typename T>
c_buffer<T>::c_buffer(c_buffer<T>&& buffer) noexcept {
    _size = buffer._size;
    _capacity = buffer._capacity;
    _head = buffer._head;
    _tail = buffer._tail;
    _mem = buffer._mem;

    buffer._mem = nullptr;
    buffer._size = 0;
    buffer._head = buffer._tail = 0;
}