cmake_minimum_required(VERSION 3.26.1)
#set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")

if(MSVC)
	if(CMAKE_BUILD_TYPE MATCHES "Debug")
		add_compile_options(/Zi /GR /EHsc /MDd /MP /Gy /JMC /fsanitize=address) 
		add_link_options(/INCREMENTAL:NO /DEBUG:FULL)
	else()
		add_compile_options(/Zi /GR /EHsc /MD /MP /Gy /JMC /fsanitize=address) 
		add_link_options(/INCREMENTAL:NO /DEBUG:FULL)
	endif()
 endif()

# 设置C++版本
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# 设置C版本
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# win32下添加宏定义
if(WIN32)
	add_definitions(-DWIN32)
	add_definitions(-D_WIN32)
	add_definitions(-DNOMINMAX)

	# add_definitions(-DUNICODE)
	# add_definitions(-D_UNICODE)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DHAVE_SNPRINTF)
	add_definitions(-DEIGEN_FORCE_INLINING)
	add_definitions(-DGLOG_NO_ABBREVIATED_SEVERITIES)
	add_definitions(-D_SILENCE_ALL_CXX23_DEPRECATION_WARNINGS)
	add_definitions(-DLEVELDB_PLATFORM_WINDOWS)

	# add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	# add_definitions(-D_SCL_SECURE_NO_WARNINGS)
	# add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
	# add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	# add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
endif()

# Enable Hot Reload for MSVC compilers if supported.
if(POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(nacos-sdk-cpp)

# if (CMAKE_VERSION VERSION_GREATER 3.12)
# set_property(TARGET nacos-sdk-cpp PROPERTY CXX_STANDARD 20)
# endif()

# add_definitions(-DNACOS_AUTH)
# add_definitions(-DNACOS_SPAS)
include_directories(
	.
	include
)

aux_source_directory(src SRCS_DIRS)
aux_source_directory(src/config SRCS_DIRS)
aux_source_directory(src/constant SRCS_DIRS)
aux_source_directory(src/crypto SRCS_DIRS)
aux_source_directory(src/crypto/base64 SRCS_DIRS)
aux_source_directory(src/crypto/hmac_sha1 SRCS_DIRS)
aux_source_directory(src/crypto/hmac_sha1/hmac SRCS_DIRS)
aux_source_directory(src/crypto/hmac_sha1/sha SRCS_DIRS)
aux_source_directory(src/crypto/md5 SRCS_DIRS)
aux_source_directory(src/factory SRCS_DIRS)
aux_source_directory(src/http SRCS_DIRS)
aux_source_directory(src/http/delegate SRCS_DIRS)
aux_source_directory(src/init SRCS_DIRS)
aux_source_directory(src/json SRCS_DIRS)
aux_source_directory(src/json/rapidjson SRCS_DIRS)
aux_source_directory(src/json/rapidjson/error SRCS_DIRS)
aux_source_directory(src/json/rapidjson/internal SRCS_DIRS)
aux_source_directory(src/json/rapidjson/msinttypes SRCS_DIRS)
aux_source_directory(src/listen SRCS_DIRS)
aux_source_directory(src/log SRCS_DIRS)
aux_source_directory(src/naming SRCS_DIRS)
aux_source_directory(src/naming/beat SRCS_DIRS)
aux_source_directory(src/naming/cache SRCS_DIRS)
aux_source_directory(src/naming/selectors SRCS_DIRS)
aux_source_directory(src/naming/subscribe SRCS_DIRS)
aux_source_directory(src/security SRCS_DIRS)
aux_source_directory(src/server SRCS_DIRS)
aux_source_directory(src/thread SRCS_DIRS)
aux_source_directory(src/utils SRCS_DIRS)
aux_source_directory(test SRCS_DIRS)
aux_source_directory(test/testcase SRCS_DIRS)

aux_source_directory(test TEST_SRCS_DIRS)
aux_source_directory(test/testcase TEST_SRCS_DIRS)

add_executable(nacos-cli.out ${TEST_SRCS_DIRS} ${SRCS_DIRS})
add_library(nacos-cli SHARED ${SRCS_DIRS})
add_library(nacos-cli-static STATIC ${SRCS_DIRS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
target_link_libraries(nacos-cli PRIVATE Threads::Threads ZLIB::ZLIB CURL::libcurl rapidjson)
target_link_libraries(nacos-cli-static PRIVATE Threads::Threads CURL::libcurl rapidjson)
target_link_libraries(nacos-cli.out PRIVATE Threads::Threads ZLIB::ZLIB CURL::libcurl rapidjson)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)
#find_package(folly REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
#find_package(absl CONFIG REQUIRED)

install(TARGETS nacos-cli
	LIBRARY DESTINATION lib)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
	DESTINATION include/nacos
	FILES_MATCHING PATTERN "*.h*")