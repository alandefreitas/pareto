//
// Created by Alan Freitas on 12/1/20.
//

#ifndef PARETO_INTERLEAVED_MEMORY_POOL_H
#define PARETO_INTERLEAVED_MEMORY_POOL_H

#include <climits>
#include <cstddef>
#include <set>
#include <cassert>

namespace pareto {

    /// \class Interleaved memory pool allocator
    ///
    /// The list of free chunks is kept inside the free chunks
    /// themselves.
    /// --->>> This is great for containers that don't need contiguous memory.
    /// --->>> This is VERY BAD for containers that need contiguous memory
    ///
    /// Each block has room for INITIAL_BLOCK_SIZE elements.
    ///
    /// When a block is full, we allocate another block for
    /// more INITIAL_BLOCK_SIZE elements.
    ///
    /// Although possible, interleaved free lists make it impractical
    /// to use it for contiguous allocation. In contiguous allocation,
    /// we need to keep the free list ordered and deallocation costs O(n).
    /// If we are using this pool for only one container, destroying
    /// the container ends up costing O(n^2).
    ///
    /// Another problem is that data structures that need contiguous
    /// allocation usually increment their demands exponentially, just
    /// like this allocator. This usually wastes a lot of space, as
    /// if we are never deallocating. It might be even worse than the
    /// O(n^2), because it wastes O(n^2) without any gain over
    /// the usual std::allocator.
    ///
    /// By default, the first block will have room for 512 elements
    /// \see https://github.com/mtrebi/memory-allocators#pool-allocator
    template<typename T = int, bool allow_contiguous_allocation = false>
    class interleaved_memory_pool {
    public:
        /// \brief Default initial block size
        /// The block has room for 512 elements, which is enough to make
        /// associative containers more efficient than contiguous containers
        /// but still doesn't waste a lot of memory for basic object types
        constexpr static size_t DEFAULT_INITIAL_BLOCK_SIZE = 512 * sizeof(T);

    public /* types */:
        typedef T value_type;
        typedef T *pointer;
        typedef T &reference;
        typedef const T *const_pointer;
        typedef const T &const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        // typedef std::false_type propagate_on_container_copy_assignment;
        // typedef std::true_type propagate_on_container_move_assignment;
        // typedef std::true_type propagate_on_container_swap;
        typedef char *binary_data_pointer_type;

        /// \class Slot for keeping memory pool objects
        /// A slot is a union might store an element or a pointer to the
        /// next free position available in the memory blocks
        union slot_type {
            /// \brief Field to store an element in the pool
            value_type element;
            /// \brief Field to store a pointer to the next free slot in the pool
            slot_type *next;

            /// \brief The constructor does not have to do anything
            slot_type() = default;

            /// \brief The destructor does not have to do anything
            /// The memory pool will deallocate positions without destructing them
            ///      (see interleaved_memory_pool::deallocate)
            /// You just need to destruct value_type if it uses dynamic memory.
            ///      In this case, destruction should happen outside the memory pool
            ///      If you need to destruct the element, you have to do it before
            ///      deallocating the memory
            /// For instance, see functions like deallocate_rstar_tree_node
            ~slot_type() = default;
        };

        // typedef slot_type slot_type_;
        typedef slot_type *slot_pointer_type;

        /// Default initial block size needs to have room for at least 2 slots
        static_assert(DEFAULT_INITIAL_BLOCK_SIZE >= 2 * sizeof(slot_type), "INITIAL_BLOCK_SIZE too small.");

        /// \class Type that represents the same allocator with a different type
        /// Some containers depend on rebind to convert the initial allocator to
        /// an allocator appropriate to their data structure. For instance,
        /// we might create a set<int> with an allocator<int>, but set<int>
        /// internally really needs is an allocator<node<int>> allocator.
        /// set<int> uses rebind so that the user does not even have to
        /// know they need allocator<node<int>>, especially because
        /// node<int> is implementation-defined and not part of the STD.
        template<typename U>
        struct rebind {
            typedef interleaved_memory_pool<U, allow_contiguous_allocation> other;
        };

    public /* constructors and destructors */:
        /// \brief Construct with all pointers to nullptr
        interleaved_memory_pool() noexcept:
                current_block_(nullptr),
                current_slot_(nullptr),
                last_slot_(nullptr),
                first_free_slot_(nullptr),
                n_blocks_(0),
                initial_block_size_(DEFAULT_INITIAL_BLOCK_SIZE) {}

        /// \brief Construct with all pointers to nullptr and set initial block size
        explicit interleaved_memory_pool(size_t initial_block_size) noexcept:
                current_block_(nullptr),
                current_slot_(nullptr),
                last_slot_(nullptr),
                first_free_slot_(nullptr),
                n_blocks_(0),
                initial_block_size_(initial_block_size) {}

        /// \brief Copy construct by setting everything to null
        /// There's no much point in copying the contents of a previous allocator
        /// At most we want to copy the type and its parameters
        interleaved_memory_pool(const interleaved_memory_pool &rhs) noexcept
                : interleaved_memory_pool(rhs.initial_block_size_) {
            // The copy constructor doesn't really copy because it wouldn't make much sense
            // to copy the data allocated for another data structure.
            // We only want to copy the parameters so it can allocate the same way.
        }

        /// \brief Move constructor: copy all pointers to this allocator
        interleaved_memory_pool(interleaved_memory_pool &&rhs) noexcept
                : initial_block_size_(rhs.initial_block_size_) {
            current_block_ = rhs.current_block_;
            rhs.current_block_ = nullptr;
            current_slot_ = rhs.current_slot_;
            last_slot_ = rhs.last_slot_;
            first_free_slot_ = rhs.first_free_slot_;
            n_blocks_ = rhs.n_blocks_;
        }

        /// \brief Construct from a memory pool of another template type
        template<class U>
        explicit interleaved_memory_pool(const interleaved_memory_pool<U> &rhs) noexcept
                : interleaved_memory_pool(rhs.initial_block_size()) {
            // The copy constructor doesn't really copy because it wouldn't make much sense
            // This simply converts the types
        }

        /// \brief Destructor to delete all blocks allocated
        virtual ~interleaved_memory_pool() noexcept {
            slot_pointer_type pointer_to_current_block = current_block_;
            while (pointer_to_current_block != nullptr) {
                slot_pointer_type pointer_to_previous_block = pointer_to_current_block->next;
                assert(pointer_to_previous_block != pointer_to_current_block);
                void *void_pointer_to_current_block = reinterpret_cast<void *>(pointer_to_current_block);
                const size_t current_block_size = initial_block_size_ << n_blocks_;
                --n_blocks_;
                operator delete(void_pointer_to_current_block, current_block_size);
                pointer_to_current_block = pointer_to_previous_block;
            }
        }

    public /* attribution operator */:
        /// \brief Copy attribution operator
        interleaved_memory_pool &operator=(const interleaved_memory_pool &memoryPool) = delete;

        /// \brief Move attribution operator
        interleaved_memory_pool &operator=(interleaved_memory_pool &&memoryPool) noexcept {
            if (this != &memoryPool) {
                std::swap(current_block_, memoryPool.current_block_);
                current_slot_ = memoryPool.current_slot_;
                last_slot_ = memoryPool.last_slot_;
                first_free_slot_ = memoryPool.first_free_slot_;
                n_blocks_ = memoryPool.n_blocks_;
                initial_block_size_ = memoryPool.initial_block_size_;
            }
            return *this;
        }

    public /* non-modifying functions */:
        /// \brief Initial block size that serves as reference for new blocks
        [[nodiscard]] size_t initial_block_size() const {
            return initial_block_size_;
        }

        /// \brief Size of the lastest block we allocated
        [[nodiscard]] size_t current_block_size() const {
            return initial_block_size_ << (n_blocks_ - 1);
        }

        /// \brief Return a pointer to the element to which the reference x refers
        pointer address(reference x) const noexcept {
            return &x;
        }

        /// \brief Return a const pointer to the element to which the reference x refers
        const_pointer address(const_reference x) const noexcept {
            return &x;
        }

        /// \brief Allocate slots
        /// Allocate 1 object in the free slots.
        ///
        /// The parameters n and hint are here to comply with the
        /// C++ specification. This allocator, however, cannot
        /// allocate more than one slot at once, and the hint
        /// is useless.
        ///
        /// You can allow allocating contiguous slots in memory but
        /// this has very bad performance for this allocator type.
        ///
        /// If the allocator does not allow allocating more than 1
        /// slot at once, this creates a logic error in your program.
        /// The allocator will not catch this error for you in release mode.
        ///
        /// Can only allocate one object at a time.
        /// n and hint are ignored because memory pools cannot
        /// efficiently guarantee a sequence of elements
        ///
        /// \param n Number of elements to be allocated contiguously
        /// \param hint Parameter ignored: only for C++ compatibility
        /// \return
        pointer allocate(size_type n = 1, [[maybe_unused]] const_pointer hint = 0) {
            if constexpr (allow_contiguous_allocation) {
                if (n == 1) {
                    return allocate_one();
                } else {
                    return allocate_many(n);
                }
            } else {
                // free list is not sorted, we can only allocate one at a time
                assert(n == 1);
                return allocate_one();
            }
        }

        /// \brief Deallocate a pointer
        /// Position p becomes the first free slot pointing
        /// to the previous first free slot
        /// This does not destroy the element
        /// You should do that yourself first
        /// \param p Pointer to element to deallocate
        /// \param n Number of contiguous elements allocated (probably 1)
        void deallocate(pointer p, size_type n = 1) {
            if constexpr (allow_contiguous_allocation) {
                deallocate_continuous(p, n);
            } else {
                // Unordered free list O(1)
                const bool pointer_is_valid = p != nullptr;
                if (pointer_is_valid) {
                    // The position now has a pointer to where we have our first free slot
                    reinterpret_cast<slot_pointer_type>(p)->next = first_free_slot_;
                    // The point p becomes the first free slot
                    first_free_slot_ = reinterpret_cast<slot_pointer_type>(p);
                }
            }
        }

        /// \brief Max number of elements in this pool
        [[nodiscard]] size_type max_size() const noexcept {
            size_type max_blocks = -1 / initial_block_size_;
            return (initial_block_size_ - sizeof(binary_data_pointer_type)) / sizeof(slot_type) * max_blocks;
        }

        /// \brief Construct new element of type U at address p
        template<class U, class... Args>
        void construct(U *p, Args &&... args) {
            // Construct new element of type U at address p
            new(p) U(std::forward<Args>(args)...);
        }

        /// \brief Destroy element of type U at position p
        template<class U>
        void destroy(U *p) {
            // Call U's destructor but leave the garbage there
            p->~U();
        }

        /// \brief Allocate space for a new element T, construct it, and return a pointer
        template<class... Args>
        pointer new_element(Args &&... args) {
            // allocate room for an element
            pointer result = allocate();
            // construct element in this address
            construct<value_type>(result, std::forward<Args>(args)...);
            // return pointer
            return result;
        }

        /// \brief Delete element of type T at position p
        void delete_element(pointer p) {
            // if p is valid
            if (p != nullptr) {
                // call its destructor
                p->~value_type();
                // deallocate address p
                deallocate(p);
            }
        }

    private:
        /// \brief Deallocate a continuous block of memory
        void deallocate_continuous(pointer p, size_type n = 1) {
            if constexpr (allow_contiguous_allocation) {
                // Free list needs to be sorted!
                // O(n) cost
                // if pointer is valid
                if (p != nullptr) {
                    // if we are deallocating before the start of the free list
                    if (comes_before(reinterpret_cast<slot_pointer_type>(p), first_free_slot_)) {
                        if (n == 1) {
                            // the position now has a pointer to where we our first free slot was
                            reinterpret_cast<slot_pointer_type>(p)->next = first_free_slot_;
                            // the point p becomes the first free slot
                            first_free_slot_ = reinterpret_cast<slot_pointer_type>(p);
                        } else {
                            for (size_t i = 0; i < n - 1; ++i) {
                                (reinterpret_cast<slot_pointer_type>(p) + i)->next = (
                                        reinterpret_cast<slot_pointer_type>(p) + i + 1);
                            }
                            (reinterpret_cast<slot_pointer_type>(p) + n - 1)->next = first_free_slot_;
                            first_free_slot_ = reinterpret_cast<slot_pointer_type>(p);
                        }
                    } else {
                        // we are deallocating after the free list beginning
                        // first_free_slot_ (free list begin) stays the same
                        // the last element in the list points to nullptr
                        // and we iterate the free list to find a place for p
                        // the element before p points to p
                        // p points to the element before p
                        slot_pointer_type cur = first_free_slot_;
                        while (cur->next != nullptr) {
                            // in an intermediary element, we want to know if it points to someone beyond p
                            // we want to know if p comes before cur->next
                            if (comes_before(reinterpret_cast<slot_pointer_type>(p), cur->next)) {
                                if (n == 1) {
                                    // if so, p should come between these elements now
                                    reinterpret_cast<slot_pointer_type>(p)->next = cur->next;
                                    cur->next = reinterpret_cast<slot_pointer_type>(p);
                                    // and we are done (in worst-case O(n))
                                    break;
                                } else {
                                    for (size_t i = 0; i < n - 1; ++i) {
                                        (reinterpret_cast<slot_pointer_type>(p) + i)->next = (
                                                reinterpret_cast<slot_pointer_type>(p) + i + 1);
                                    }
                                    (reinterpret_cast<slot_pointer_type>(p) + n - 1)->next = cur->next;
                                    cur->next = reinterpret_cast<slot_pointer_type>(p);
                                    break;
                                }
                            }
                            // if the element didn't point to someone beyond p
                            // we move to the next
                            cur = cur->next;
                        }
                        // if we got to the last element (we didn't break the loop)
                        if (cur->next == nullptr) {
                            // last element now points to p
                            cur->next = reinterpret_cast<slot_pointer_type>(p);
                            // elements inside p point to the next
                            for (size_t i = 0; i < n - 1; ++i) {
                                (reinterpret_cast<slot_pointer_type>(p) + i)->next = (
                                        reinterpret_cast<slot_pointer_type>(p) + i + 1);
                            }
                            // p is now the last element and points to nullptr
                            reinterpret_cast<slot_pointer_type>(p + n - 1)->next = nullptr;
                        }
                    }
                }
            }
        }

        /// \brief Find a contiguous block of free memory
        /// This is *very* expensive. It's why we don't use this allocator
        /// for contiguous memory
        pointer find_n_contiguous(size_type n) {
            slot_pointer_type first_free_slot_last = first_free_slot_;
            slot_pointer_type contiguous_start = first_free_slot_last;
            slot_pointer_type before_contiguous_start = nullptr;
            size_t n_contiguous_found = 0;

            for (; first_free_slot_last->next != nullptr; first_free_slot_last = first_free_slot_last->next) {
                ++n_contiguous_found;
                const bool found_n_contiguous = n_contiguous_found == n;
                if (found_n_contiguous) {
                    break;
                }

                const bool next_element_is_not_contiguous = first_free_slot_last->next > first_free_slot_last + 1;
                if (next_element_is_not_contiguous) {
                    n_contiguous_found = 0;
                    contiguous_start = first_free_slot_last->next;
                    before_contiguous_start = first_free_slot_last;
                }
            }

            bool last_is_not_counted_yet = first_free_slot_last->next == nullptr;
            if (last_is_not_counted_yet) {
                ++n_contiguous_found;
            }

            // did we find n contiguous elements?
            const bool found_n_contiguous = n_contiguous_found == n;
            if (found_n_contiguous) {
                const bool update_the_list = before_contiguous_start == nullptr;
                if (update_the_list) {
                    first_free_slot_ = first_free_slot_last->next;
                } else {
                    before_contiguous_start = first_free_slot_last->next;
                }
                return reinterpret_cast<pointer>(first_free_slot_last);
            }
            return nullptr;
        }

        /// \brief Allocate a block of n contiguous elements in the current block or a new block
        /// This is the function we call for contiguous allocation if we cannot find
        /// a contiguous block of free memory
        pointer allocate_another_block(size_type n) {
            slot_pointer_type first_free_slot_last = first_free_slot_;
            // all elements from current_slot_ to last_slot_
            // become part of the free list
            // if list is not empty
            const bool there_are_free_slots = first_free_slot_ != nullptr;
            if (there_are_free_slots) {
                // finish iterating to the free list end
                while (first_free_slot_last->next != nullptr) {
                    first_free_slot_last = first_free_slot_last->next;
                }
                // all positions from current slot to last_slot go to the free list
                first_free_slot_last->next = current_slot_;
                first_free_slot_last = first_free_slot_last->next;
                while (first_free_slot_last < last_slot_) {
                    first_free_slot_last->next = first_free_slot_last + 1;
                    ++first_free_slot_last;
                    ++current_slot_;
                }
                first_free_slot_last->next = nullptr;
            } else if (current_slot_ != nullptr) {
                // if free list was empty
                // it now starts at the current slot
                first_free_slot_ = current_slot_;
                // we iterate until last slot making it part of
                // the free list
                slot_pointer_type i = current_slot_;
                while (i < last_slot_) {
                    i->next = i + 1;
                    ++i;
                    ++current_slot_;
                }
                i->next = nullptr;
            }
            // allocate another block of elements
            allocate_block();

            // if there is no room even in this new block
            // try everything again
            if (current_slot_ + n > last_slot_) {
                return allocate(n);
            } else {
                // if there is room in the new block
                slot_pointer_type res = current_slot_;
                current_slot_ += n;
                return reinterpret_cast<pointer>(res);
            }
        }

        /// \brief Allocate n contiguous blocks of memory
        /// This will try to find n contiguous free slots.
        /// If this is not possible, we look for contiguous memory
        ///       at the end of the current block
        /// If this is not possible, we create a new block and allocate
        ///       contiguous memory there
        pointer allocate_many(size_type n) {
            // If there are free slots
            const bool there_are_free_slots = first_free_slot_ != nullptr;
            if (there_are_free_slots) {
                pointer pointer_to_contiguous_memory = find_n_contiguous(n);
                if (pointer_to_contiguous_memory) {
                    return pointer_to_contiguous_memory;
                }
            }

            bool we_need_another_block = current_slot_ + n > last_slot_;
            if (we_need_another_block) {
                return allocate_another_block(n);
            } else {
                // Just move the current slot forward
                slot_pointer_type res = current_slot_;
                current_slot_ += n;
                return reinterpret_cast<pointer>(res);
            }
        }

        /// \brief Allocate a single slot
        /// This is the common use case for this allocator
        pointer allocate_one() {
            const bool there_are_free_slots = first_free_slot_ != nullptr;
            if (there_are_free_slots) {
                auto result = reinterpret_cast<pointer>(first_free_slot_);
                first_free_slot_ = first_free_slot_->next;
                return result;
            } else {
                const bool we_have_no_block_yet = current_slot_ == nullptr && last_slot_ == nullptr;
                const bool we_need_another_block = current_slot_ > last_slot_;
                if (we_have_no_block_yet || we_need_another_block) {
                    allocate_block();
                }
                // move current slot forward
                // return pointer to current slot, which is where we
                // keep the element in lack of a free_slot_ != nullptr
                return reinterpret_cast<pointer>(current_slot_++);
            }
        }

        /// \brief True if address "a" comes before address "b" in the pool
        /// Note that this considers not only the address but also which
        /// blocks we created first.
        /// This keeps an order relationship that allows us to keep free
        /// chunks ordered.
        /// This is an important subproblem of contiguous allocation,
        /// and one of the many reasons why it's very efficient.
        bool comes_before(slot_pointer_type a, slot_pointer_type b) {
            // for our purposes, nullptr represents +inf because the last element points to it
            if (b == nullptr) {
                return true;
            }
            if (a == nullptr) {
                return false;
            }

            // Iterate blocks and block sizes
            slot_pointer_type curr = current_block_;
            auto n_blocks = n_blocks_;
            // If current block is not null
            while (curr != nullptr) {
                // Are a or b in this block?
                bool a_in_block = (curr <= a && a < curr + (initial_block_size_ << (n_blocks - 1)));
                bool b_in_block = (curr <= a && a < curr + (initial_block_size_ << (n_blocks - 1)));

                if (a_in_block) {
                    if (b_in_block) {
                        return a < b;
                    } else {
                        return false;
                    }
                } else if (b_in_block) {
                    return true;
                }

                // If none of them in this block, try next block
                --n_blocks;
                curr = curr->next;
            }

            throw std::invalid_argument("We can't find the pointers in any of the blocks");
        }

        /// \brief Pad pointer to respect the alignment required by T
        size_type pad_pointer(binary_data_pointer_type p, size_type align) const noexcept {
            // Get p as uint
            auto p_as_int = reinterpret_cast<uintptr_t>(p);
            // Return aligned size
            return ((align - p_as_int) % align);
        }

        /// \brief Allocate space for a new block and store a pointer to the previous one
        void allocate_block() {
            // Size of this new block grows exponentially with number of blocks
            const size_t block_size = initial_block_size_ << n_blocks_;

            // Keep track of the number of blocks so we can increase
            // the size of new blocks exponentially
            ++n_blocks_;

            // create a new block of char* with block size
            auto new_block = reinterpret_cast<binary_data_pointer_type>
            (operator new(block_size));

            // this first slot of a block is a special slot that points to the previous block
            // - interpret this as a pointer to slot and set its next field to current block
            // - the first slot of a new block will point to the previous block
            // - if this is the first block being allocated, the previous block is nullptr
            reinterpret_cast<slot_pointer_type>(new_block)->next = current_block_;

            // current block object member points to this new block
            // - we need this to keep track of blocks when we need to destroy them
            current_block_ = reinterpret_cast<slot_pointer_type>(new_block);

            // Pad block body to satisfy the alignment requirements for elements
            // the block body is everything after the first pointer to previous block

            // Body starts after one slot size
            binary_data_pointer_type body = new_block + sizeof(slot_pointer_type);

            // Get size in bytes we need to align the body to a slot_type
            // - The second slot might need to start a little after sizeof(slot_pointer_type) bytes
            size_type body_padding = pad_pointer(body, alignof(slot_type));

            // Current slot points to first address of the body
            // We will keep moving this slot forward whenever the user asks
            // to allocate something
            current_slot_ = reinterpret_cast<slot_pointer_type>(body + body_padding);

            // Last slot points to last position available in the block
            last_slot_ = reinterpret_cast<slot_pointer_type>
            (new_block + block_size - sizeof(slot_type));
        }

    private:
        /// Pointer to where the current block begins
        /// When we run out of space, we allocate a new block
        /// This slot pointer next member points to the
        /// previous block
        slot_pointer_type current_block_;

        /// Slot where we should allocate the next element
        /// If the user doesn't deallocate anything, first_free_slot_
        /// will be nullptr, and we start allocating at current_slot_
        /// This starts as the first slot of current block
        slot_pointer_type current_slot_;

        /// Last slot of current block
        /// When the current slot gets there, we allocate a new block
        slot_pointer_type last_slot_;

        /// First free slot
        /// It only becomes != nullptr when the user deallocates
        /// something. Otherwise, we allocate from current slot.
        /// The free slots are not ordered. This makes it hard to
        /// allocate contiguous memory.
        slot_pointer_type first_free_slot_;

        /// Number of blocks in this allocator
        size_t n_blocks_{0};

        /// Initial block size
        size_t initial_block_size_{DEFAULT_INITIAL_BLOCK_SIZE};

    };
}

#endif //PARETO_INTERLEAVED_MEMORY_POOL_H
