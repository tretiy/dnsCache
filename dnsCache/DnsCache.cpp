#include "DnsCache.h"
#include <exception>
#include <iostream>
#include <mutex>
#include <ostream>
#include <shared_mutex>

DNSCache::DNSCache(size_t max_size) : maxSize(max_size) {
  nameMap.reserve(max_size);
  std::cout << "DNSCache ctor " << max_size << "\n";
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
      try {
        updElem = new iplist_t(name, ip);
        nameMap.insert({name, updElem});
      } catch (std::exception &ex) {
        delete updElem;
        std::cerr << "Could not insert new dns entry:" << ex.what()
                  << std::endl;
        throw; // may be we could try to decrease cnt of elements to fit new one
               // and as a result max_size will be smaller than user set
      }
    }
  }
  moveToHead(updElem);
}

std::string DNSCache::resolve(const std::string &name) {
  iplist_t *ipElem = nullptr;
  {
    std::shared_lock mapReadLock(mapMutex);
    if (nameMap.contains(name)) {
      ipElem = nameMap[name];
    }
  }
  if (ipElem)
    moveToHead(ipElem);
  return ipElem ? ipElem->ip : "";
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