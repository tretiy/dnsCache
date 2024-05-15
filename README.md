# DNSCache
## To build this project 
- set $VCPKG_ROOT variable to vcpkg directory
- run cmake --workflow --preset "LinuxWorkflow"
## Operations complexity
- Resolve O(1) (search in hash table and update DL list)
- Update O(1) (insert/search in hash table and update DL list)
## Cache as singleton
- Use DNSCacheManager to ensure that only one instance of DNSCache exists
## Open questions
- Out of memory exception (For now dnscache just log it to cerr and rethrow exception)
- DNSCacheManager always returns dnsCache object with initially created max_size
