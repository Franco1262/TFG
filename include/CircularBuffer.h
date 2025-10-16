#pragma once
#include <stdint.h>
#include <stddef.h>

template<typename T, int Size>
class CircularBuffer 
{
    private:
        T buffer[Size];
        int head;
        int tail;
        int count;

    public:
        void clear() 
        {
            head = 0;
            tail = 0;
            count = 0;
        }

        bool full() const { return count == Size; }

        bool empty() const { return count == 0; }

        int size() const { return count; }

        bool push(T item) 
        {
            if (full()) return false;
            
            buffer[head] = item;
            head = (head + 1) % Size;
            count++;
            return true;
        }

        bool pop(T &item) 
        {
            if (count == 0) return false;
            
            item = buffer[tail];
            tail = (tail + 1) % Size;
            count--;
            return true;
        }

        bool discard()       // remove only
        {
            if (empty()) return false;

            tail = (tail + 1) % Size;
            count--;
            return true;
        }

        void discard(size_t n)
        {
            if (n == 0 || empty()) return;

            int toDiscard = n > static_cast<size_t>(count) ? count : static_cast<int>(n);
            tail = (tail + toDiscard) % Size;
            count -= toDiscard;
        }
        
        struct Iterator 
            {
                const T* ptr_buffer;
                int current_pos;
                int items_checked;
                int capacity;

                Iterator& operator++() 
                {
                    current_pos = (current_pos + 1) % capacity;
                    items_checked++;
                    return *this;
                }

                bool operator!=(const Iterator& other) const 
                {
                    return items_checked != other.items_checked;
                }

                const T& operator*() const { return ptr_buffer[current_pos]; }
            };

            Iterator begin() const { return {buffer, tail, 0, Size}; }
            Iterator end()   const { return {buffer, 0, count, Size}; }
            

        struct MutableIterator 
        {
            T* ptr_buffer;
            int current_pos;
            int items_checked;
            int capacity;
            MutableIterator& operator++() { current_pos = (current_pos + 1) % capacity; items_checked++; return *this; }
            bool operator!=(const MutableIterator& other) const { return items_checked != other.items_checked; }
            T& operator*() { return ptr_buffer[current_pos]; }
        };

        MutableIterator begin() { return {buffer, tail, 0, Size}; }
        MutableIterator end()   { return {buffer, 0, count, Size}; }
};
