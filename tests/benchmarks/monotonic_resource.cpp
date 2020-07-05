// <memory_resource> implementation -*- C++ -*-

// Copyright (C) 2018 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// Under Section 7 of GPL version 3, you are granted additional
// permissions described in the GCC Runtime Library Exception, version
// 3.1, as published by the Free Software Foundation.

// You should have received a copy of the GNU General Public License and
// a copy of the GCC Runtime Library Exception along with this program;
// see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
// <http://www.gnu.org/licenses/>.

#include "monotonic_resource.h"
#include <algorithm>
#include <atomic>
#include <new>
#if ATOMIC_POINTER_LOCK_FREE != 2
# include <bits/std_mutex.h>	// std::mutex, std::lock_guard
# include <bits/move.h>		// std::exchange
#endif

namespace std __attribute__ ((visibility ("default")))
{
namespace pmr
{
    namespace
    {
        class newdel_res_t final : public memory_resource
        {
            void*
            do_allocate(size_t __bytes, size_t __alignment) override
            { return ::operator new(__bytes, std::align_val_t(__alignment)); }

            void
            do_deallocate(void* __p, size_t __bytes, size_t __alignment) noexcept
            override
            { ::operator delete(__p, __bytes, std::align_val_t(__alignment)); }

            bool
            do_is_equal(const memory_resource& __other) const noexcept override
            { return &__other == this; }
        };

        class null_res_t final : public memory_resource
        {
            void*
            do_allocate(size_t, size_t) override
            { std::__throw_bad_alloc(); }

            void
            do_deallocate(void*, size_t, size_t) noexcept override
            { }

            bool
            do_is_equal(const memory_resource& __other) const noexcept override
            { return &__other == this; }
        };

        template<typename T>
        struct constant_init
        {
            union {
                unsigned char unused;
                T obj;
            };
            constexpr constant_init() : obj() { }

            template<typename U>
            explicit constexpr constant_init(U arg) : obj(arg) { }

            ~constant_init() { /* do nothing, union member is not destroyed */ }
        };

        constant_init<newdel_res_t> newdel_res{};
        constant_init<null_res_t> null_res{};
#if ATOMIC_POINTER_LOCK_FREE == 2
        using atomic_mem_res = atomic<memory_resource*>;
# define _GLIBCXX_ATOMIC_MEM_RES_CAN_BE_CONSTANT_INITIALIZED
#elif defined(_GLIBCXX_HAS_GTHREADS)
        // Can't use pointer-width atomics, define a type using a mutex instead:
    struct atomic_mem_res
    {
# ifdef __GTHREAD_MUTEX_INIT
#  define _GLIBCXX_ATOMIC_MEM_RES_CAN_BE_CONSTANT_INITIALIZED
      // std::mutex has constexpr constructor
      constexpr
# endif
      atomic_mem_res(memory_resource* r) : val(r) { }

      mutex mx;
      memory_resource* val;

      memory_resource* load()
      {
	lock_guard<mutex> lock(mx);
	return val;
      }

      memory_resource* exchange(memory_resource* r)
      {
	lock_guard<mutex> lock(mx);
	return std::exchange(val, r);
      }
    };
#else
# define _GLIBCXX_ATOMIC_MEM_RES_CAN_BE_CONSTANT_INITIALIZED
    // Single-threaded, no need for synchronization
    struct atomic_mem_res
    {
      constexpr
      atomic_mem_res(memory_resource* r) : val(r) { }

      memory_resource* val;

      memory_resource* load() const
      {
	return val;
      }

      memory_resource* exchange(memory_resource* r)
      {
	return std::exchange(val, r);
      }
    };
#endif // ATOMIC_POINTER_LOCK_FREE == 2

#ifdef _GLIBCXX_ATOMIC_MEM_RES_CAN_BE_CONSTANT_INITIALIZED
        constant_init<atomic_mem_res> default_res{&newdel_res.obj};
#else
# include "default_resource.h"
#endif
    } // namespace

    memory_resource*
    new_delete_resource() noexcept
    { return &newdel_res.obj; }

    memory_resource*
    null_memory_resource() noexcept
    { return &null_res.obj; }

    memory_resource*
    set_default_resource(memory_resource* r) noexcept
    {
        if (r == nullptr)
            r = new_delete_resource();
        return default_res.obj.exchange(r);
    }

    memory_resource*
    get_default_resource() noexcept
    { return default_res.obj.load(); }

    // Member functions for std::pmr::monotonic_buffer_resource

    // Memory allocated by the upstream resource is managed in a linked list
    // of _Chunk objects. A _Chunk object recording the size and alignment of
    // the allocated block and a pointer to the previous chunk is placed
    // at end of the block.
    class monotonic_buffer_resource::_Chunk
    {

        template<typename _Tp>
        constexpr int
        static __countl_zero(_Tp __x) noexcept
        {
            constexpr auto _Nd = numeric_limits<_Tp>::digits;

            if (__x == 0)
                return _Nd;

            constexpr auto _Nd_ull = numeric_limits<unsigned long long>::digits;
            constexpr auto _Nd_ul = numeric_limits<unsigned long>::digits;
            constexpr auto _Nd_u = numeric_limits<unsigned>::digits;

            if constexpr (_Nd <= _Nd_u)
            {
                constexpr int __diff = _Nd_u - _Nd;
                return __builtin_clz(__x) - __diff;
            }
            else if constexpr (_Nd <= _Nd_ul)
            {
                constexpr int __diff = _Nd_ul - _Nd;
                return __builtin_clzl(__x) - __diff;
            }
            else if constexpr (_Nd <= _Nd_ull)
            {
                constexpr int __diff = _Nd_ull - _Nd;
                return __builtin_clzll(__x) - __diff;
            }
            else // (_Nd > _Nd_ull)
            {
                static_assert(_Nd <= (2 * _Nd_ull),
                              "Maximum supported integer size is 128-bit");

                unsigned long long __high = __x >> _Nd_ull;
                if (__high != 0)
                {
                    constexpr int __diff = (2 * _Nd_ull) - _Nd;
                    return __builtin_clzll(__high) - __diff;
                }
                constexpr auto __max_ull = numeric_limits<unsigned long long>::max();
                unsigned long long __low = __x & __max_ull;
                return (_Nd - _Nd_ull) + __builtin_clzll(__low);
            }
        }


        template<typename _Tp>
        constexpr _Tp
        static __ceil2(_Tp __x) noexcept
        {
            constexpr auto _Nd = numeric_limits<_Tp>::digits;
            if (__x == 0 || __x == 1)
                return 1;
            const unsigned __n = _Nd - __countl_zero((_Tp)(__x - 1u));
            const _Tp __y_2 = (_Tp)1u << (__n - 1u);
            return __y_2 << 1u;
        }

    template<typename _Tp, typename _Up, bool = is_integral_v<_Tp>>
    struct _If_is_unsigned_integer_type { };

  template<typename _Up>
    struct _If_is_unsigned_integer_type<bool, _Up, true> { };

  template<typename _Tp, typename _Up>
    struct _If_is_unsigned_integer_type<_Tp, _Up, true>
    : enable_if<is_same_v<_Tp, make_unsigned_t<_Tp>>, _Up> { };

  template<typename _Tp, typename _Up = _Tp>
    using _If_is_unsigned_integer
      = typename _If_is_unsigned_integer_type<remove_cv_t<_Tp>, _Up>::type;

//#if ! __STRICT_ANSI__
  // [bitops.rot], rotating

  template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp>
    rotl(_Tp __x, unsigned int __s) noexcept
    { return __rotl(__x, __s); }

  template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp>
    rotr(_Tp __x, unsigned int __s) noexcept
    { return __rotr(__x, __s); }

  // [bitops.count], counting

        template<typename _Tp>
        constexpr _Tp
        __rotl(_Tp __x, unsigned int __s) noexcept
        {
            constexpr auto _Nd = numeric_limits<_Tp>::digits;
            const unsigned __sN = __s % _Nd;
            return (__x << __sN) | (__x >> ((_Nd - __sN) % _Nd));
        }

        template<typename _Tp>
        constexpr _Tp
        __rotr(_Tp __x, unsigned int __s) noexcept
        {
            constexpr auto _Nd = numeric_limits<_Tp>::digits;
            const unsigned __sN = __s % _Nd;
            return (__x >> __sN) | (__x << ((_Nd - __sN) % _Nd));
        }

        template<typename _Tp>
        constexpr int
        __countl_one(_Tp __x) noexcept
        {
            if (__x == numeric_limits<_Tp>::max())
                return numeric_limits<_Tp>::digits;
            return __countl_zero<_Tp>((_Tp)~__x);
        }

        template<typename _Tp>
        constexpr int
        __countr_zero(_Tp __x) noexcept
        {
            constexpr auto _Nd = numeric_limits<_Tp>::digits;

            if (__x == 0)
                return _Nd;

            constexpr auto _Nd_ull = numeric_limits<unsigned long long>::digits;
            constexpr auto _Nd_ul = numeric_limits<unsigned long>::digits;
            constexpr auto _Nd_u = numeric_limits<unsigned>::digits;

            if constexpr (_Nd <= _Nd_u)
            return __builtin_ctz(__x);
            else if constexpr (_Nd <= _Nd_ul)
            return __builtin_ctzl(__x);
            else if constexpr (_Nd <= _Nd_ull)
            return __builtin_ctzll(__x);
            else // (_Nd > _Nd_ull)
            {
                static_assert(_Nd <= (2 * _Nd_ull),
                              "Maximum supported integer size is 128-bit");

                constexpr auto __max_ull = numeric_limits<unsigned long long>::max();
                unsigned long long __low = __x & __max_ull;
                if (__low != 0)
                    return __builtin_ctzll(__low);
                unsigned long long __high = __x >> _Nd_ull;
                return __builtin_ctzll(__high) + _Nd_ull;
            }
        }

        template<typename _Tp>
        constexpr int
        __countr_one(_Tp __x) noexcept
        {
            if (__x == numeric_limits<_Tp>::max())
                return numeric_limits<_Tp>::digits;
            return __countr_zero((_Tp)~__x);
        }


        template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp, int>
    countl_zero(_Tp __x) noexcept
    { return __countl_zero(__x); }

  template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp, int>
    countl_one(_Tp __x) noexcept
    { return __countl_one(__x); }

  template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp, int>
    countr_zero(_Tp __x) noexcept
    { return __countr_zero(__x); }

  template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp, int>
    countr_one(_Tp __x) noexcept
    { return __countr_one(__x); }

  template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp, int>
    popcount(_Tp __x) noexcept
    { return __popcount(__x); }
//#endif

  // Integral power-of-two operations

        template<typename _Tp>
        constexpr bool
        __ispow2(_Tp __x) noexcept
        { return std::__popcount(__x) == 1; }


        template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp, bool>
    ispow2(_Tp __x) noexcept
    { return __ispow2(__x); }

  template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp>
    ceil2(_Tp __x) noexcept
    { return __ceil2(__x); }

        template<typename _Tp>
        constexpr _Tp
        __floor2(_Tp __x) noexcept
        {
            constexpr auto _Nd = numeric_limits<_Tp>::digits;
            if (__x == 0)
                return 0;
            return (_Tp)1u << (_Nd - __countl_zero((_Tp)(__x >> 1)));
        }


        template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp>
    floor2(_Tp __x) noexcept
    { return __floor2(__x); }

        template<typename _Tp>
        constexpr _Tp
        static __log2p1(_Tp __x) noexcept
        {
            constexpr auto _Nd = numeric_limits<_Tp>::digits;
            return _Nd - __countl_zero(__x);
        }

  template<typename _Tp>
    constexpr _If_is_unsigned_integer<_Tp>
    log2p1(_Tp __x) noexcept
    { return __log2p1(__x); }

    public:
        // Return the address and size of a block of memory allocated from __r,
        // of at least __size bytes and aligned to __align.
        // Add a new _Chunk to the front of the linked list at __head.
        static pair<void*, size_t>
        allocate(memory_resource* __r, size_t __size, size_t __align,
                 _Chunk*& __head)
        {
            __size = __ceil2(__size + sizeof(_Chunk));
            void* __p = __r->allocate(__size, __align);
            // Add a chunk defined by (__p, __size, __align) to linked list __head.
            void* const __back = (char*)__p + __size - sizeof(_Chunk);
            __head = ::new(__back) _Chunk(__size, __align, __head);
            return { __p, __size - sizeof(_Chunk) };
        }

        // Return every chunk in linked list __head to resource __r.
        static void
        release(_Chunk*& __head, memory_resource* __r) noexcept
        {
            _Chunk* __next = __head;
            __head = nullptr;
            while (__next)
            {
                _Chunk* __ch = __next;
                __builtin_memcpy(&__next, __ch->_M_next, sizeof(_Chunk*));

                assert(__ch->_M_canary != 0);
                assert(__ch->_M_canary == (__ch->_M_size|__ch->_M_align));

                if (__ch->_M_canary != (__ch->_M_size | __ch->_M_align))
                    return; // buffer overflow detected!

                size_t __size = (1u << __ch->_M_size);
                size_t __align = (1u << __ch->_M_align);
                void* __start = (char*)(__ch + 1) - __size;
                __r->deallocate(__start, __size, __align);
            }
        }

    private:
        _Chunk(size_t __size, size_t __align, _Chunk* __next) noexcept
                : _M_size(__log2p1(__size) - 1),
                  _M_align(__log2p1(__align) - 1)
        {
            __builtin_memcpy(_M_next, &__next, sizeof(__next));
            _M_canary = _M_size | _M_align;
        }

        unsigned char _M_canary;
        unsigned char _M_size;
        unsigned char _M_align;
        unsigned char _M_next[sizeof(_Chunk*)];
    };

    void
    monotonic_buffer_resource::_M_new_buffer(size_t bytes, size_t alignment)
    {
        // Need to check this somewhere, so put it here:
        static_assert(alignof(monotonic_buffer_resource::_Chunk) == 1);

        const size_t n = std::max(bytes, _M_next_bufsiz);
        const size_t m = std::max(alignment, alignof(std::max_align_t));
        auto [p, size] = _Chunk::allocate(_M_upstream, n, m, _M_head);
        _M_current_buf = p;
        _M_avail = size;
        _M_next_bufsiz *= _S_growth_factor;
    }

    void
    monotonic_buffer_resource::_M_release_buffers() noexcept
    {
        _Chunk::release(_M_head, _M_upstream);
    }

} // namespace pmr
} // namespace std

