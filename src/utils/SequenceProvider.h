#pragma once
#include <sys/stat.h>
#include <fcntl.h>
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
#include <io.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include  <istream>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include <execution>

#include "NacosString.h"
#include "NacosExceptions.h"
#include "thread/AtomicInt.h"
#include "src/thread/Mutex.h"
#include "src/config/IOUtils.h"

namespace nacos
{
	template <typename T>
	class SequenceProvider
	{
	private:
		NacosString _fileName;
		AtomicInt<T> _current;
		Mutex _acquireMutex;
		T _nrToPreserve;
		T _initSequence;
		volatile T _hwm; //high water mark

		void ensureWrite(int fd, T data)
		{
			unsigned int bytes_written = 0;

			std::fstream file(_fileName.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			while (file.is_open())
			{
				if (file.write(reinterpret_cast<char*>(&data), sizeof(T))) {
						file.close();
				        break;
				}
			    if (file.fail() && !file.eof()) {
			        // handle the write error
			        file.clear();
			    }
			}
			//while (bytes_written < sizeof(T))
			//{
			//	#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
			//		const int written_s = _write(fd, reinterpret_cast<char*>(&data) + bytes_written, sizeof(T) - bytes_written);
			//		if (written_s == -1)
			//		{
			//			if (errno == EINTR)
			//			{
			//				//retry the write operation
			//			}
			//			else
			//			{
			//				//perror(TEXT(""));
			//				//handle the error
			//			}
			//		}
			//		else if (written_s < sizeof(T) - bytes_written)
			//		{
			//			bytes_written += written_s;
			//			//handle incomplete write
			//		}
			//		else
			//		{
			//			bytes_written += written_s;
			//		}
			//	#else
	  //          bytes_written += write(fd, (char*)&data + bytes_written, sizeof(T) - bytes_written);
			//	#endif
			//}
		}

		T preserve()
		{
#if defined(_MSC_VER) || defined(__WIN32__) || defined(WIN32)
			T current;
			bool newFile = false;
			if (IOUtils::checkNotExistOrNotFile(_fileName))
			{
				newFile = true;
			}
			int fd = -1;
			if (const errno_t err = _sopen_s(&fd, _fileName.c_str(), _O_RDWR | _O_CREAT, _SH_DENYNO,
			                                 _S_IREAD | _S_IWRITE); err != 0)
			{
				throw NacosException(NacosException::UNABLE_TO_OPEN_FILE, _fileName);
			}

			if (newFile)
			{
				ensureWrite(fd, _initSequence);
				if (const auto pos = _lseek(fd, 0L, SEEK_SET); pos == -1L)
					perror("_lseek to end failed");
			}

			unsigned int bytes_read = 0;
			//todo dead lock
			int bytesReads;

			std::fstream file(_fileName.c_str(), std::ios::in | std::ios::out | std::ios::binary);
			while (file.is_open())
			{
				;
				if (file.read(reinterpret_cast<char*>(&current), sizeof(T))) {
						file.close();
				        break;
				}
			    if (file.fail() && !file.eof()) {
			        // handle the write error
			        ensureWrite(fd, _initSequence);
			    }
			}


			//while (bytes_read < sizeof(T))
			//{
			//	if ((bytesReads = _read(fd, reinterpret_cast<char*>(&current) + bytes_read, sizeof(T) - bytes_read)) <=
			//		0)
			//	{
			//		if (bytesReads == 0)
			//		{
			//			std::this_thread::yield();
			//		}
			//		//perror(std::to_string(sizeof(T)).c_str());
			//		//throw std::exception("Problem reading file");
			//	}
			//	bytes_read += bytesReads;
			//}

			if (const auto pos = _lseek(fd, 0L, SEEK_SET); pos == -1L)
				perror("_lseek to end failed");
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
            throw new NacosException(NacosException::UNABLE_TO_OPEN_FILE, _fileName);
        }
        fd = open(_fileName.c_str(), O_RDWR | O_CREAT, mode);
        
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
		SequenceProvider(const NacosString& fileName, T initSequence, T nr_to_preserve)
		{
			_fileName = fileName;
			_initSequence = initSequence;
			_nrToPreserve = nr_to_preserve;
			_current.set(preserve());
		}

		T next(int step = 1)
		{
			T res = _current.getAndInc(step);
			while (res >= _hwm)
			{
				_acquireMutex.lock();
				if (res >= _hwm)
				{
					preserve();
				}
				_acquireMutex.unlock();
			}
			return res;
		}
	};
} // namespace nacos
