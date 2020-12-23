//
// Created by Alan Freitas on 12/16/20.
//

#ifndef PARETO_DEFAULT_ALLOCATOR_H
#define PARETO_DEFAULT_ALLOCATOR_H

#include <iostream>
#ifdef BUILD_PARETO_WITH_PMR
#include <memory>
#include <memory_resource>
#else
#include <memory>
#endif

namespace pareto {
    /// \brief Tells if an allocator is a pmr allocator
    template <typename> struct is_polymorphic_allocator : std::false_type {};

#ifdef BUILD_PARETO_WITH_PMR
    /// \brief Tells if an allocator is a pmr allocator (when PMR is available)
    template <typename T>
    struct is_polymorphic_allocator<std::pmr::polymorphic_allocator<T>>
        : std::true_type {};

    /// \brief Alias to default allocator for spatial containers
    template <class T>
    using default_allocator_type = std::pmr::polymorphic_allocator<T>;

    class placeholder_memory_resource : public std::pmr::memory_resource {
        void *do_allocate(std::size_t bytes, std::size_t alignment) override {
            throw std::logic_error("You cannot allocate memory with a "
                                   "placeholder memory resource");
        };

        void do_deallocate(void *p, std::size_t bytes,
                           std::size_t alignment) override {
            throw std::logic_error("You cannot deallocate memory with a "
                                   "placeholder memory resource");
        };

        [[nodiscard]] bool do_is_equal(
            const std::pmr::memory_resource &other) const noexcept override {
            return false;
        };
    };

    /// \brief Create a pmr placeholder allocator
    /// This is a placeholder to be replaced with an allocator
    /// using the internal memory pool as resource
    template <class allocator_type> allocator_type placeholder_allocator() {
        if constexpr (is_polymorphic_allocator<allocator_type>::value) {
            static placeholder_memory_resource p;
            return allocator_type(&p);
        } else {
            return allocator_type();
        }
    }

    template <class ALLOC> bool is_placeholder_allocator(const ALLOC &alloc) {
        if constexpr (is_polymorphic_allocator<ALLOC>::value) {
            auto resource_as_placeholder =
                dynamic_cast<placeholder_memory_resource *>(
                    alloc.resource());
            return resource_as_placeholder != nullptr;
        } else {
            return false;
        }
    }
#else
    /// \brief Alias to default allocator for spatial containers
    template <class T> using default_allocator_type = std::allocator<T>;

    template <class allocator_type> allocator_type placeholder_allocator() {
        return allocator_type();
    }

    template <class ALLOC> bool is_placeholder_allocator(const ALLOC &) {
        return false;
    }
#endif

    /// \class Traits for a container capable of managing its own memory pool
    /// A container that inherits from container_with_pool will have its
    /// own memory pool (if PMR is available). If no allocator is provided
    /// at construction, then we will create a default allocator which
    /// uses this pool (if PMR is available). This is important in spatial
    /// containers to avoid one allocation per node.
    class container_with_pool {
      public:
#ifdef BUILD_PARETO_WITH_PMR
        /// \brief Create the allocator for the container
        /// This is only used as default allocator, if no allocator
        /// is provided.
        template <class ALLOC> ALLOC construct_allocator(const ALLOC& alloc) {
            if constexpr (is_polymorphic_allocator<ALLOC>::value) {
                if (is_placeholder_allocator(alloc)) {
                    memory_pool_ =
                        std::make_unique<std::pmr::unsynchronized_pool_resource>();
                    return ALLOC(memory_pool_.get());
                }
                return ALLOC(alloc);
            } else {
                return ALLOC(alloc);
            }
        }

        std::unique_ptr<std::pmr::unsynchronized_pool_resource> memory_pool_{
            nullptr};
#else
        template <class ALLOC> ALLOC construct_allocator(const ALLOC& alloc) {
            return ALLOC(alloc);
        }
#endif
    };

} // namespace pareto

#endif // PARETO_DEFAULT_ALLOCATOR_H
