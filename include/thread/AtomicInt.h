#ifndef ATOMIC_INT_H_
#define ATOMIC_INT_H_

namespace nacos
{
    #include <atomic>
    template <typename T>
    class AtomicInt
    {
        std::atomic<T> _curval;

    public:
        AtomicInt(T curval = 0) : _curval(curval){}

        void set(T val) { _curval = val; }

        T inc(T incval = 1)
        {
            T oldValue = getAndInc(incval);
            return oldValue + incval;
        }

        T getAndInc(T incval = 1)
        {
            T oldValue = _curval.fetch_add(incval);
            return oldValue;
        }

        T dec(const int decval = 1)
        {
            return inc(-decval);
        }

        T get() const { return _curval; }
    };
} // namespace nacos

#endif
