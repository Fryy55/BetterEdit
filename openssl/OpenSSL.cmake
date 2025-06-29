
# find_package(OpenSSL REQUIRED)

# message("OpenSSL version: ${OPENSSL_VERSION}")

# target_link_libraries(${PROJECT_NAME} OpenSSL::SSL)
# target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto)

message(STATUS "Using my awesome personal OpenSSL libs!")

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

if (WIN32)
    file(GLOB OPENSSL_LIBS ${CMAKE_CURRENT_LIST_DIR}/lib/win64/*.lib)
elseif (ANDROID)
    if (CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        file(GLOB OPENSSL_LIBS ${CMAKE_CURRENT_LIST_DIR}/lib/android64/*.a)
    else()
        file(GLOB OPENSSL_LIBS ${CMAKE_CURRENT_LIST_DIR}/lib/android32/*.a)
    endif()
elseif(IOS)
    file(GLOB OPENSSL_LIBS ${CMAKE_CURRENT_LIST_DIR}/lib/ios/*.a)
elseif(APPLE)
    file(GLOB OPENSSL_LIBS ${CMAKE_CURRENT_LIST_DIR}/lib/macos/*.a)
else()
    message(FATAL_ERROR "OpenSSL libs not on this platform")
endif()

message(STATUS "Linking OpenSSL libs: ${OPENSSL_LIBS}")

target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBS})
