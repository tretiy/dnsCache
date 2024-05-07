#include "DnsCache.h"

DNSCache::DNSCache(size_t max_size) : maxSize(max_size)
{
    /* Not implemented yet */
}

DNSCache::~DNSCache()
{}

void DNSCache::update(const std::string &name, const std::string &ip)
{
    std::scoped_lock Lock(dataMutex);
    if (nameMap.contains(name)) {
        nameMap[name]->ip = ip;
    } else {
        if (nameMap.size() >= maxSize) {
            nameMap.erase(ipListTail->name);
            if (ipListTail->prev)
                ipListTail->prev->next = nullptr;
            auto newTail = ipListTail->prev;
            delete ipListTail;
            ipListTail = newTail;
        }
        iplist_t *newElem = new iplist_t(name, ip);
        nameMap.insert({name, newElem});
        moveToHead(newElem);
    }
}

std::string DNSCache::resolve(const std::string &name)
{
    std::scoped_lock Lock(dataMutex);
    if (nameMap.contains(name)) {
        auto ipElem = nameMap[name];
        moveToHead(ipElem);
        return ipElem->ip;
    }
    return {};
}
void DNSCache::moveToHead(iplist_t *elem)
{
    if (ipListTail == elem) {
        if (ipListTail->prev) {
            ipListTail = ipListTail->prev;
            ipListTail->next = nullptr;
        }
    }
    if (!ipListTail)
        ipListTail = elem;

    if (elem->prev)
        elem->prev->next = elem->next;
    if (elem->next)
        elem->next->prev = elem->prev;

    elem->prev = nullptr;
    elem->next = ipListHead;
    if (ipListHead)
        ipListHead->prev = elem;

    if (!ipListTail)
        ipListTail = elem;
}