#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

template <class NameT, class IpT> struct DLType {
  DLType(const NameT &name, const IpT &ip)
      : name(name), ip(ip), prev(nullptr), next(nullptr){};
  NameT name;
  IpT ip;
  DLType *prev;
  DLType *next;
};

class DNSCache {
private:
  explicit DNSCache(size_t max_size);
  DNSCache(const DNSCache &other) = delete;
  ~DNSCache();

public:
  void update(const std::string &name, const std::string &ip);
  std::string resolve(const std::string &name);
  typedef DLType<std::string, std::string> iplist_t;

private:
  const size_t maxSize;
  iplist_t *ipListHead = nullptr;
  iplist_t *ipListTail = nullptr;
  std::unordered_map<std::string, iplist_t *> nameMap;
  mutable std::shared_mutex mapMutex;
  mutable std::mutex listMutex;

  void moveToHead(iplist_t *elem);

  friend class DNSCacheManager;
  friend class DNSCacheForTests;
};
struct DNSCacheManager {
  static DNSCache &GetCacheInstance(size_t max_size) {
    static DNSCache cacheInst(max_size);
    return cacheInst;
  }
};
struct DNSCacheForTests {
  DNSCacheForTests(size_t max_size) : cache(max_size){};
  DNSCache cache;
};
/*
Класс хранит в себе соответствия имя = ip адрес, максимальное число
записей, которые он может хранить задается в конструкторе через
max_size.
Через update() в класс поступают новые данные, которые могут или
обновить существующую записть или добавить новую.
При превышении лимита max_size из памяти должны удаляться самые
давно неиспользуемые записи.
Метод resolve() возвращает из кеша IP адрес для имени name или пустую
строку, если не найден.
Класс должен обеспечивать корректную работу в многопоточном
приложении, когда update() и resolve() вызываются из разных ниток
параллельно.
Вопросы:
1. Предложите работающую без ошибок реализацию класса,
обеспечивающую максимальное быстродействие.

2. Какую сложность обеспечивает предложенное решение при вставке
записей, при обновлении существующих записей, при поиске записей?

3. Доработайте класс, измените при необходимости его интерфейс, чтобы
гарантировать существование только одного экземпляра класса в
процессе.
Результатом хотелось бы видеть компилируемые и работающие
исходники под g++ / Linux
*/