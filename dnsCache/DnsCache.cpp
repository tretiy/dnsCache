#include "DnsCache.h"
#include <exception>
#include <iostream>
#include <mutex>
#include <shared_mutex>

DNSCache::DNSCache(size_t max_size) : maxSize(max_size) {
  nameMap.reserve(max_size);
}

DNSCache::~DNSCache() {
  while (ipListHead) {
    auto elem = ipListHead;
    ipListHead = elem->next;
    delete elem;
  }
}

void DNSCache::update(const std::string &name, const std::string &ip) {

  iplist_t *updElem;
  {
    std::unique_lock mapWriteLock(mapMutex);
    if (nameMap.contains(name)) {
      updElem = nameMap[name];
      updElem->ip = ip;
    } else {
      if (nameMap.size() >= maxSize) {
        std::scoped_lock listWriteLock(listMutex);
        nameMap.erase(ipListTail->name);
        auto newTail = ipListTail->prev;
        delete ipListTail;
        ipListTail = newTail;
        if (ipListTail)
          ipListTail->next = nullptr;
      }
      updElem = new (std::nothrow) iplist_t(name, ip);
      if (!updElem)
        std::cerr << "Could not allocate new dns entry";
      else {
        try {
          nameMap.insert({name, updElem});
        } catch (std::exception &ex) {
          delete updElem;
          std::cerr << "Could not insert new dns entry";
        }
      }
    }
  }
  moveToHead(updElem);
}

std::string DNSCache::resolve(const std::string &name) {
  iplist_t *ipElem = nullptr;
  std::shared_lock mapReadLock(mapMutex);
  if (nameMap.contains(name)) {
    ipElem = nameMap[name];
  }
  if (ipElem)
    moveToHead(ipElem);
  return ipElem ? ipElem->ip : "";
}

void DNSCache::printState() {
  std::cout << "DNSCache Current state...\n"
            << "max size is " << maxSize << "\n elems in map " << nameMap.size()
            << "\n";
  std::cout << "Print first 10 elements\n";
  int i = 10;
  for (auto &[key, value] : nameMap) {
    std::cout << value->name << "   " << value->ip << "\n";
    if (i-- == 0)
      break;
  }
}
void DNSCache::moveToHead(iplist_t *elem) {
  if (!elem)
    return;
  std::scoped_lock listWriteLock(listMutex);
  // check if tail and head initialized
  if (!ipListTail) {
    ipListTail = elem;
  }
  if (!ipListHead) {
    ipListHead = elem;
  }

  // check is elem a head or a tail
  if (ipListHead == elem)
    return;
  if (ipListTail == elem) {
    if (ipListTail->prev) {
      ipListTail = ipListTail->prev;
      ipListTail->next = nullptr;
    }
  }

  // update prev and next  elements
  if (elem->prev)
    elem->prev->next = elem->next;
  if (elem->next)
    elem->next->prev = elem->prev;

  // move to head
  elem->prev = nullptr;
  elem->next = ipListHead;
  ipListHead->prev = elem;
  ipListHead = elem;
}