//
// Created by chris on 7/21/2025.
//

#ifndef MEMORYALLOCATOR_H
#define MEMORYALLOCATOR_H

#include "DataTypes.h"
#include "Log/LogManager.h"

////////////////////////////////////////////////
// Memory Allocator Interface
////////////////////////////////////////////////

/**
 * @class MemoryAllocator
 * @brief Interface for various memory allocation strategies
 *
 * This abstract base class defines the interface for memory allocators
 * that can be used in the application. Derived classes must provide
 * concrete implementations for memory management.
 */
class MemoryAllocator {
public:
    MemoryAllocator(const mla_string_t& p_Name) {
        m_Name = p_Name;
    }
public:

    /**
     * @brief Allocates a memory block of the specified size
     * @param size Size of memory to Allocate in bytes
     * @return Pointer to the allocated memory region or nullptr on failure
     */
    virtual mla_pointer_t Allocate(mla_size_t size);

    /**
     * @brief Deallocates a previously allocated memory block
     * @param ptr Pointer to the memory region to free
     */
    virtual void Deallocate(mla_pointer_t ptr);

    /**
     * @brief Resets the allocator to its initial state
     */
    virtual void Reset();

    /**
     * @brief Determines the amount of currently used memory
     * @return Number of bytes in use
     */
    virtual mla_uint64_t GetUsedMemory();

    /**
     * @brief Determines the total amount of available memory
     * @return Total size in bytes
     */
    virtual mla_uint64_t GetTotalMemory();

    /**
     * @brief Gets the name of the allocator
     * @return Name of the allocator
     */
    mla_string_t getName() const {
        return m_Name;
    }

private:
    mla_string_t m_Name; // Name of the allocator, can be used for debugging or logging
};

#define mla_new(allocator, type) static_cast<type*>(new(allocator->Allocate(sizeof(type))) type)
#define mla_new_array(allocator, type, size) static_cast<type*>(allocator->Allocate(sizeof(type) * size))
#define mla_delete(allocator, ptr) allocator->Deallocate(static_cast<mla_pointer_t>(ptr))



/**
 * @class DefaultMemoryAllocator
 * @brief Default memory allocator using global new/delete operators
 *
 * This class implements the MemoryAllocator interface using the global
 * new and delete operators for memory management. It maintains a linked list
 * of allocated memory chunks to track allocations and deallocations.
 */

struct DefaultMemoryAllocatorChunk {
    mla_pointer_t ptr; // Pointer to the allocated memory
    mla_size_t size; // Size of the allocated memory chunk
    DefaultMemoryAllocatorChunk* last; // Pointer to the last chunk in the linked list
};

class DefaultMemoryAllocator : public MemoryAllocator {
public:
    DefaultMemoryAllocator(const mla_string_t& p_Name): MemoryAllocator(p_Name) {
    }

    DefaultMemoryAllocator(const mla_string_t& p_Name, mla_int64_t p_MemoryLimit): MemoryAllocator(p_Name) {
        m_MemoryLimit = p_MemoryLimit;
    }

    ~DefaultMemoryAllocator() {
        Reset(); // Clean up all allocated memory chunks
    }

public:

    mla_pointer_t Allocate(mla_size_t size) override {
        if (size == 0) {
            return nullptr; // Handle zero allocation gracefully
        }

        if (m_UsedMemory + size > m_MemoryLimit) {
            mla_string_t l_Name = getName();
            LogError("DefaultMemoryAllocator + " + String(l_Name.data, l_Name.length) + ": Allocation failed, size: " + String(size) + ", used: " + String(m_UsedMemory) + ", limit: " + String(m_MemoryLimit));
            return nullptr; // Handle out of memory condition
        }

        mla_pointer_t ptr = ::operator new(size); // Use global new operator
        if (ptr == nullptr) {
            mla_string_t l_Name = getName();
            LogError("DefaultMemoryAllocator + " + String(l_Name.data, l_Name.length) + ": Allocation failed, size: " + String(size));
            return nullptr; // Handle allocation failure
        }
        mla_memset(ptr, 0, size);

        // Create a new chunk and add it to the linked list
        DefaultMemoryAllocatorChunk* newChunk = new DefaultMemoryAllocatorChunk();
        newChunk->ptr = ptr;
        newChunk->last = m_Head; // Link the new chunk to the previous head
        m_Head = newChunk; // Update the head to the new chunk
        m_UsedMemory += size; // Update the total used memory


        return ptr;
    }

    void Deallocate(mla_pointer_t ptr) override {
        if (ptr != nullptr) {
            ::operator delete(ptr); // Use global delete operator
        }

        // Find the chunk in the linked list and remove it
        DefaultMemoryAllocatorChunk* current = m_Head;
        DefaultMemoryAllocatorChunk* previous = nullptr;
        while (current != nullptr) {
            if (current->ptr == ptr) {
                // Found the chunk to deallocate
                if (previous != nullptr) {
                    previous->last = current->last; // Link previous to next
                } else {
                    m_Head = current->last; // Update head if it's the first chunk
                }
                delete current; // Free the chunk memory
                break; // Exit after deallocating
            }
            previous = current;
            current = current->last; // Move to the next chunk
        }
    }

    void Reset() override {

        // Reset the allocator by deallocating all chunks
        DefaultMemoryAllocatorChunk* current = m_Head;
        while (current != nullptr) {
            DefaultMemoryAllocatorChunk* next = current->last; // Store the next chunk
            ::operator delete(current->ptr); // Deallocate the memory
            delete current; // Delete the chunk structure
            current = next; // Move to the next chunk
        }

    }

    mla_uint64_t GetUsedMemory() override {

        return m_UsedMemory;
    }
    mla_uint64_t GetTotalMemory() override {
        return m_MemoryLimit; // Limited only by the os
    }
private:
    DefaultMemoryAllocatorChunk* m_Head = nullptr; // Pointer to the head of the linked list of allocated chunks
    mla_uint64_t m_UsedMemory = 0; // Total used memory, not strictly necessary for this implementation
    mla_uint64_t m_MemoryLimit = mla_uint64_max; // Memory limit for the allocator, default is maximum size
};

/* * @class ArenaAllocator
 * @brief Memory allocator that uses a fixed-size arena for allocations
 *
 * This class implements the MemoryAllocator interface using a fixed-size arena.
 * It allows for fast allocations and deallocations within the arena, but does not
 * support individual deallocation of memory blocks.
 */


class ArenaAllocator: public MemoryAllocator {
public:

    ArenaAllocator(const mla_string_t& p_Name, mla_size_t p_Size): ArenaAllocator(p_Name, p_Size, nullptr) {
        // Constructor that initializes the arena with a specified size and no parent allocator
    }

    ArenaAllocator(const mla_string_t& p_Name, mla_size_t p_Size, MemoryAllocator* p_ParentAllocator): MemoryAllocator(p_Name) {
        m_ArenaSize = p_Size;
        m_UsedMemory = 0;

        if (p_ParentAllocator == nullptr) {
            m_ParentAllocator = nullptr;
            m_ArenaStart = ::operator new(m_ArenaSize);
            mla_memset(m_ArenaStart, 0, m_ArenaSize);// Allocate memory for the arena using global new operator
        } else {
            m_ParentAllocator = p_ParentAllocator;
            m_ArenaStart = p_ParentAllocator->Allocate(m_ArenaSize); // Allocate memory for the arena using the parent allocator
        }

    }

    ~ArenaAllocator() {

        if (m_ArenaStart == nullptr) {
            return; // Nothing to deallocate
        }

        if (m_ParentAllocator == nullptr) {
            ::operator delete(m_ArenaStart); // Use global delete operator if no parent allocator
        } else {
            m_ParentAllocator->Deallocate(m_ArenaStart);
            m_ParentAllocator = nullptr; // Clear the parent allocator pointer
        }

        m_ArenaStart = nullptr; // Clear the arena start pointer
        m_ArenaSize = 0; // Reset the arena size
        m_UsedMemory = 0; // Reset the used memory

    }

    mla_pointer_t Allocate(mla_size_t size) override {
        if (size == 0 || m_UsedMemory + size > m_ArenaSize) {
            mla_string_t l_Name = getName();
            LogError("ArenaAllocator + " + String(l_Name.data, l_Name.length) +": Allocation failed, size: " + String(size) + ", used: " + String(m_UsedMemory) + ", total: " + String(m_ArenaSize));
            return nullptr; // Handle zero allocation or out of memory
        }
        mla_pointer_t ptr = static_cast<mla_pointer_t>(static_cast<mla_uint8_t*>(m_ArenaStart) + m_UsedMemory);
        m_UsedMemory += size; // Update used memory
        return ptr;
    }

    void Deallocate(mla_pointer_t ptr) override {
        // In an arena allocator, deallocation is not supported.
        // You can implement a reset method to clear the arena if needed.
    }

    mla_uint64_t GetUsedMemory() override {
        return m_UsedMemory; // Return the amount of used memory
    }
    mla_uint64_t GetTotalMemory() override {
        return m_ArenaSize; // Return the total size of the arena
    }

    void Reset() {
        // Reset and clear the arena with 0
        mla_memset(m_ArenaStart, 0, m_UsedMemory);
        m_UsedMemory = 0;
    }
private:
    mla_pointer_t m_ArenaStart;
    mla_size_t m_ArenaSize;
    mla_size_t m_UsedMemory;
    MemoryAllocator* m_ParentAllocator;
};


#endif
