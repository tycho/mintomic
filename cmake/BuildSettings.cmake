if(${MSVC})
    # Enable debug info in Release.
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /debug")
elseif(${UNIX})
    if(${CYGWIN})
        set(CMAKE_C_FLAGS "-g")
        set(CMAKE_CXX_FLAGS "-g")
    else()
        # These are all required on Xcode 4.5.1 + iOS, because the defaults are no good.
        set(CMAKE_C_FLAGS "-pthread -g")
        set(CMAKE_CXX_FLAGS "-pthread -g")
    endif()
    set(CMAKE_C_FLAGS_DEBUG "-O0")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm.*$")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wa,-mimplicit-it=always")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mimplicit-it=always")
endif()
endif()

if(${IOS})
    set(CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")
    set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
endif()
