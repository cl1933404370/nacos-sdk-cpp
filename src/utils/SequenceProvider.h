#pragma once
#include <sys/stat.h>
#include <fcntl.h>
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include "NacosString.h"
#include "NacosExceptions.h"
#include "thread/AtomicInt.h"
#include "src/thread/Mutex.h"
#include "src/config/IOUtils.h"

namespace nacos
{

template<typename T>
class SequenceProvider {
private:
    NacosString _fileName;
    AtomicInt<T> _current;
    Mutex _acquireMutex;
    T _nrToPreserve;
    T _initSequence;
    volatile T _hwm;//high water mark

    static void ensureWrite(int fd, T data) {
    	unsigned int bytes_written = 0;
        while (bytes_written < sizeof(T)) {
            #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
            bytes_written += _write(fd, reinterpret_cast<char*>(&data) + bytes_written, sizeof(T) - bytes_written);
            #else
            bytes_written += write(fd, (char*)&data + bytes_written, sizeof(T) - bytes_written);
            #endif
        }
    }
    
    T preserve() {
        #if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
        T current{};
        bool newFile = false;
        if (IOUtils::checkNotExistOrNotFile(_fileName)) {
            newFile = true;
        }
        int fd = -1;
        errno_t err = _sopen_s( &fd, _fileName.c_str(), _O_RDONLY, _SH_DENYNO,
                          _S_IREAD | _S_IWRITE );
        if (err != 0 ) {
            throw NacosException(NacosException::UNABLE_TO_OPEN_FILE, _fileName);
        }

        if (newFile) {
            ensureWrite(fd, _initSequence);
            _lseek(fd, 0, SEEK_SET);//read from the beginning
        }

        unsigned int bytes_read = 0;
        //todo dead lock
        int bytes_reads = 0;
        while (bytes_read < sizeof(T))
        {
            if (( bytes_reads = _read( fd, &current + bytes_read, sizeof(T) - bytes_read )) <= 0 )
            {
	            perror( std::to_string(sizeof(T)).c_str() );
                throw "Problem reading file";
            }
			bytes_read += bytes_reads;
        }

        _lseek(fd, 0, SEEK_SET);//write from the beginning
        ensureWrite(fd, current + _nrToPreserve);
        _close(fd);
        _hwm = current + _nrToPreserve;
        return current;
        #else
        T current;
        int fd;
        bool newFile = false;
        if (IOUtils::checkNotExistOrNotFile(_fileName)) {
            newFile = true;
        }
        mode_t mode = S_IRUSR | S_IWUSR | S_IRWXG | S_IWGRP;
        fd = open(_fileName.c_str(), O_RDWR | O_CREAT, mode);
        if (fd <= 0) {
            throw new NacosException(NacosException::UNABLE_TO_OPEN_FILE, _fileName);
        }
        
        if (newFile) {
            ensureWrite(fd, _initSequence);
            lseek(fd, 0, SEEK_SET);//read from the beginning
        }

        size_t bytes_read = 0;
        while (bytes_read < sizeof(T))
        {
            bytes_read += read(fd, (char*)&current + bytes_read, sizeof(T) - bytes_read);
        }
        lseek(fd, 0, SEEK_SET);//write from the beginning

        ensureWrite(fd, current + _nr_to_preserve);
        close(fd);
        _hwm = current + _nr_to_preserve;
        return current;
        #endif
    }
public:
    SequenceProvider(const NacosString &fileName, T initSequence, T nr_to_preserve) {
        _fileName = fileName;
        _initSequence = initSequence;
        _nrToPreserve = nr_to_preserve;
        _current.set(preserve());
    }

    T next(int step = 1) {
        T res = _current.getAndInc(step);
        while (res >= _hwm) {
            _acquireMutex.lock();
            if (res >= _hwm) {
                preserve();
            }
            _acquireMutex.unlock();
        }
        return res;
    }
};

} // namespace nacos
