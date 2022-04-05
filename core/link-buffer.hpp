//
// Created by tyx on 4/4/22.
//

#ifndef TINYRPC_LINK_BUFFER_HPP
#define TINYRPC_LINK_BUFFER_HPP

struct LinkBufferNode {
  // buffer
  char* buf;
  // read-offset
  int off;
  // write-offset
  int malloc;
  // reference count
  int refer;
  // read-only node, introduced by Refer,WriteString, WriteBinary, etc., default
  // false
  bool readonly;
  LinkBufferNode* origin;
  // the next node of the linked buffer
  LinkBufferNode* next;
};

class LinkBuffer {
 private:
  int length_;
  int malloc_size_;
  LinkBufferNode* head_;
  LinkBufferNode* read_;
  LinkBufferNode* flush_;
  LinkBufferNode* write_;

  void* cache_;
};

#endif  // TINYRPC_LINK_BUFFER_HPP
