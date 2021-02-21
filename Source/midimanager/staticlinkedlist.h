// Copyright 2021 Charles Julian Knight
// https://github.com/rabidaudio/midi-voicesteal
#ifndef LIB_MIDIMANAGER_STATICLINKEDLIST_H_
#define LIB_MIDIMANAGER_STATICLINKEDLIST_H_

#include <assert.h>
#include "./types.h"

#define SLL_ASSERT_SANITY_CHECKS() \
    assert((size_ == 0) == (cur_ == nullptr)); \
    if (TSize > 1) { \
      assert(head_->up == nullptr); \
      assert(tail_->down == nullptr); \
      assert(cur_ == nullptr || cur_->up != cur_->down); \
    }

template <typename T>
struct Node {
  T data;
  Node<T>* up;
  Node<T>* down;
};

template <typename T>
class Iterator {
 private:
  Node<T>* index_;
 public:
  explicit Iterator(Node<T>* start) {
    index_ = start;
  }

  bool hasNext() {
    return index_ != nullptr;
  }

  T& next() {
    T* value = &index_->data;
    index_ = index_->down;
    return *value;
  }
};

// StaticLinkedList is a linked list implementation which
// uses no dynamic allocations. It's max size is determined
// at compile time by TSize. If it's max size is exceeded, it
// will forget elements at the end of the list.
// It has 2 push implementations, allowing it to be used
// as a stack or as a queue (or a combination).
// It also has the ability to remove elements from arbitrary
// positions in the list. All operations are constant time
// except for arbitrary access and removal, which are linear.
// This class is *not* thread-safe.
// TSize must be positive.
template <typename T, size_t TSize> class StaticLinkedList {
 private:
  Node<T> nodes_[TSize];
  Node<T>* head_;  // the max slot
  Node<T>* tail_;  // the min slot
  Node<T>* cur_;   // the top of the stack, from tail
  size_t size_;

 public:
  StaticLinkedList() {
    assert(TSize > 0);
    cur_ = nullptr;
    size_ = 0;
    tail_ = nodes_;
    head_ = nodes_ + TSize - 1;
    for (size_t i = 1; i < TSize-1; i++) {
      nodes_[i].up = nodes_ + i + 1;
      nodes_[i].down = nodes_ + i - 1;
    }
    tail_->down = nullptr;
    head_->up = nullptr;
    if (TSize == 1) {
      tail_->up = nullptr;
      head_->down = nullptr;
      return;
    }
    tail_->up = nodes_ + 1;
    head_->down = nodes_ + TSize - 2;
    SLL_ASSERT_SANITY_CHECKS();
  }

  ~StaticLinkedList() {}

  bool isEmpty() {
    SLL_ASSERT_SANITY_CHECKS();
    return size_ == 0;
  }

  size_t size() {
    SLL_ASSERT_SANITY_CHECKS();
    return size_;
  }

  void clear() {
    size_ = 0;
    cur_ = nullptr;
    SLL_ASSERT_SANITY_CHECKS();
  }

  // Index into an arbitrary position in the list. If you want
  // to loop through all elements, use the iterator instead. O(n)
  T& operator[](size_t index) {
    Iterator<T> it = iterator();
    for (size_t i = 0; i < index; i++) {
      it.next();
    }
    return it.next();
  }

  // iterator provides a method to iterate through the items
  // in the list in order. Note that if you mutate the list during
  // the iteration, the iterator becomes invalid, and continuing to
  // use it results in undefined behavior.
  Iterator<T> iterator() {
    return Iterator<T>(cur_);
  }

  // pushStack adds an item to the top, i.e. the next item to be popped.
  // Use this if you wish to use the list as a stack. If the stack is full,
  // it will forget the oldest item in the stack. O(1).
  void pushStack(const T item) {
    if (cur_ == nullptr) {
      tail_->data = item;
      cur_ = tail_;
      size_ = 1;
      SLL_ASSERT_SANITY_CHECKS();
      return;
    }
    if (TSize == 1) {
      cur_->data = item;
      SLL_ASSERT_SANITY_CHECKS();
      return;
    }
    if (cur_->up == nullptr) {
      // we're full, so steal from the bottom
      freeToHead(tail_);
      size_--;
    }
    // now there's definitely space
    cur_ = cur_->up;
    cur_->data = item;
    size_++;
    SLL_ASSERT_SANITY_CHECKS();
    return;
  }

  // pushQueue adds the item to the bottom, i.e. the last
  // item to be popped. Use this if you which to use the list
  // as a queue. Steals from the top if the list is full. O(1).
  void pushQueue(const T item) {
    if (isEmpty() || TSize == 1) {
      pushStack(item);
      return;
    }
    if (cur_ == head_) {
      size_--;
      cur_ = head_->down;
    }
    freeToTail(head_);
    tail_->data = item;
    size_++;
    SLL_ASSERT_SANITY_CHECKS();
    return;
  }

  // look at the next item without chaning the list.
  T& peek() {
    return cur_->data;
  }

  T& pop() {
    T* result = &cur_->data;
    // this will set cur_ to nullptr if we're at tail, which is expected
    cur_ = cur_->down;
    size_--;
    SLL_ASSERT_SANITY_CHECKS();
    return *result;
  }

  // removeAt removes elements from anywhere in the list.
  // index zero is the top of the stack. O(n)
  void removeAt(size_t index) {
    if (index >= size_) return;  // outside bounds
    if (index == 0) {
      pop();
      return;
    }
    Node<T>* item = cur_;
    for (size_t i = 0; i < index; i++) {
      item = item->down;
    }
    freeToHead(item);
    size_--;
  }

 private:
  void freeToTail(Node<T>* item) {
    if (item->up == nullptr) {
      head_ = head_->down;
      head_->up = nullptr;
    } else {
      item->down->up = item->up;
      item->up->down = item->down;
    }
    tail_->down = item;
    item->up = tail_;
    item->down = nullptr;
    tail_ = item;
    SLL_ASSERT_SANITY_CHECKS();
  }

  void freeToHead(Node<T>* item) {
    // remove this one from the chain
    if (item->down == nullptr) {
      tail_ = tail_->up;
      tail_->down = nullptr;
    } else {
      item->down->up = item->up;
      item->up->down = item->down;
    }
    // put this one at the head
    head_->up = item;
    item->down = head_;
    item->up = nullptr;
    head_ = item;
    SLL_ASSERT_SANITY_CHECKS();
  }
};

#endif  // LIB_MIDIMANAGER_STATICLINKEDLIST_H_
