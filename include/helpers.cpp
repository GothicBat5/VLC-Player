#ifndef VLC_CXX_HELPERS_HPP
#define VLC_CXX_HELPERS_HPP


#if defined(__cplusplus) || defined(DOC)

#include <memory>
#include <utility>
#include <type_traits>
#include <string>
#include <stdexcept>

#ifdef VLC_THREADS_H_
# include <vlc_interrupt.h>
#endif

namespace vlc
{
namespace
{
template <typename T, typename Releaser>inline auto wrap_cptr( T* ptr, Releaser&& r ) noexcept -> std::unique_ptr<T, typename std::decay<decltype( r )>::type>
{
    return std::unique_ptr<T, typename std::decay<decltype( r )>::type>{
                ptr, std::forward<Releaser>( r )
    };
}

template <typename T, typename Releaser>
inline auto wrap_carray( T* ptr, Releaser&& r ) noexcept -> std::unique_ptr<T[], typename std::decay<decltype( r )>::type>
{
    return std::unique_ptr<T[], typename std::decay<decltype( r )>::type>{
        ptr, std::forward<Releaser>( r )
    };
}


template <typename T>inline std::unique_ptr<T, void (*)(void*)> wrap_cptr( T* ptr ) noexcept
{
    return wrap_cptr( ptr, &free );
}

template <typename T>
inline std::unique_ptr<T[], void (*)(void*)> wrap_carray( T* ptr ) noexcept
{
    return wrap_carray( ptr, &free );
}

} // anonymous namespace

template <typename T, auto HOLD, auto RELEASE>
class vlc_shared_data_ptr {
    T *ptr = nullptr;

public:
    /* default implicit constructor */
    vlc_shared_data_ptr() = default;

    explicit vlc_shared_data_ptr(T *ptr, bool hold = true)
        : ptr(ptr)
    {
        if (ptr && hold)
            HOLD(ptr);
    }

    vlc_shared_data_ptr(std::nullptr_t)
        : ptr(nullptr)
    {
    }

    vlc_shared_data_ptr(const vlc_shared_data_ptr &other)
        : vlc_shared_data_ptr(other.ptr) {}

    vlc_shared_data_ptr(vlc_shared_data_ptr &&other) noexcept
        : ptr(other.ptr)
    {
        other.ptr = nullptr;
    }

    ~vlc_shared_data_ptr()
    {
        if (ptr) RELEASE(ptr);
    }

    vlc_shared_data_ptr &operator=(const vlc_shared_data_ptr &other)
    {
        reset(other.ptr, true);
        return *this;
    }

    vlc_shared_data_ptr &operator=(vlc_shared_data_ptr &&other) noexcept
    {
        reset(other.ptr, false);
        other.ptr = nullptr;
        return *this;
    }

    bool operator==(const vlc_shared_data_ptr &other) const
    {
        return ptr == other.ptr;
    }

    bool operator==(std::nullptr_t) const noexcept
    {
        return ptr == nullptr;
    }

    bool operator!=(const vlc_shared_data_ptr &other) const
    {
        return !(*this == other);
    }

    bool operator!=(std::nullptr_t) const noexcept
    {
        return ptr != nullptr;
    }

    explicit operator bool() const
    {
        return ptr;
    }

    T &operator*() const
    {
        return *ptr;
    }

    T *operator->() const
    {
        return ptr;
    }

    T *get() const
    {
        return ptr;
    }

    void reset(T *newptr = nullptr, bool hold = true)
    {
        if (newptr && hold)
            HOLD(newptr);
        if (ptr)
            RELEASE(ptr);
        ptr = newptr;
    }
};

#if defined(VLC_THREADS_H_) || defined(DOC)

namespace threads
{

class mutex
{
public:
    mutex() noexcept
    {
        vlc_mutex_init( &m_mutex );
    }

    mutex( const mutex& ) = delete;
    mutex& operator=( const mutex& ) = delete;
    mutex( mutex&& ) = delete;
    mutex& operator=( mutex&& ) = delete;

    void lock() noexcept
    {
        vlc_mutex_lock( &m_mutex );
    }
    void unlock() noexcept
    {
        vlc_mutex_unlock( &m_mutex );
    }

private:
    vlc_mutex_t m_mutex;
    friend class condition_variable;
    friend class mutex_locker;
};

class condition_variable
{
public:
    condition_variable() noexcept
    {
        vlc_cond_init( &m_cond );
    }
    void signal() noexcept
    {
        vlc_cond_signal( &m_cond );
    }
    void broadcast() noexcept
    {
        vlc_cond_broadcast( &m_cond );
    }
    void wait( mutex& mutex ) noexcept
    {
        vlc_cond_wait( &m_cond, &mutex.m_mutex );
    }
    int timedwait( mutex& mutex, vlc_tick_t deadline ) noexcept
    {
        return vlc_cond_timedwait( &m_cond, &mutex.m_mutex, deadline );
    }

private:
    vlc_cond_t m_cond;
};

class mutex_locker
{
public:
    mutex_locker( vlc_mutex_t* m ) noexcept
        : m_mutex( m )
    {
        vlc_mutex_lock( m_mutex );
    }
    mutex_locker( mutex& m ) noexcept
        : mutex_locker( &m.m_mutex )
    {
    }
    ~mutex_locker()
    {
        vlc_mutex_unlock( m_mutex );
    }
    mutex_locker( const mutex_locker& ) = delete;
    mutex_locker& operator=( const mutex_locker& ) = delete;
    mutex_locker( mutex_locker&& ) = delete;
    mutex_locker& operator=( mutex_locker&& ) = delete;

private:
    vlc_mutex_t* m_mutex;
};

class semaphore
{
public:
    semaphore() noexcept
    {
        vlc_sem_init( &m_sem, 0 );
    }
    semaphore( unsigned int count ) noexcept
    {
        vlc_sem_init( &m_sem, count );
    }
    ~semaphore()
    {
    }

    semaphore( const semaphore& ) = delete;
    semaphore& operator=( const semaphore& ) = delete;
    semaphore( semaphore&& ) = delete;
    semaphore& operator=( semaphore&& ) = delete;

    int post() noexcept
    {
        return vlc_sem_post( &m_sem );
    }
    void wait() noexcept
    {
        vlc_sem_wait( &m_sem );
    }

    int wait_i11e() noexcept
    {
        return vlc_sem_wait_i11e( &m_sem );
    }

private:
    vlc_sem_t m_sem;
};

}

#endif // VLC_THREADS_H_

#ifdef VLC_URL_H

class url : public vlc_url_t
{
public:
    class invalid : public std::runtime_error
    {
    public:
        explicit invalid( const char* url ) : std::runtime_error( std::string{ "Invalid url: " } + url )
        {
          
        }
    };

    url()
    {
        psz_buffer = nullptr;
        psz_pathbuffer = nullptr;
        psz_host = nullptr;
    }

    explicit url( const char* str )
    {
        if ( vlc_UrlParse( this, str ) )
        {
            vlc_UrlClean( this );
            throw invalid( str );
        }
    }

    explicit url( const std::string& str) : url( str.c_str() )
    {
    }

    ~url()
    {
        vlc_UrlClean(this);
    }

    url( const url& ) = delete;
    url& operator=( const url& ) = delete;

    url( url&& u ) noexcept
        : vlc_url_t( u )
    {
        u.psz_buffer = nullptr;
        u.psz_pathbuffer = nullptr;
        u.psz_host = nullptr;
    }

    url& operator=( url&& u ) noexcept
    {
        vlc_UrlClean( this );
        *(static_cast<vlc_url_t*>( this )) = u;
        u.psz_buffer = nullptr;
        u.psz_pathbuffer = nullptr;
        u.psz_host = nullptr;
        return *this;
    }
};

#endif

} // namespace vlc

#endif
#endif // VLC_CXX_HELPERS_HPP
