check_cxx_compiler_flag(-std=c++11 CXX_FLAG_CXX11)
if (CXX_FLAG_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else(CXX_FLAG_CXX11)
  check_cxx_compiler_flag(-std=c++03 CXX_FLAG_CXX03)
  if (CXX_FLAG_CXX03)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03")
  else(CXX_FLAG_CXX03)
    check_cxx_compiler_flag(-std=c++98 CXX_FLAG_CXX98)
    if (CXX_FLAG_CXX98)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
    else(CXX_FLAG_CXX98)
    endif(CXX_FLAG_CXX98)
  endif(CXX_FLAG_CXX03)
endif(CXX_FLAG_CXX11)

set(test_flags
    -pedantic -Wall -Wcast-align -Wwrite-strings -Wswitch-default
    -Wcast-qual -Wunused-variable -Wredundant-decls
    -Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder -Wold-style-cast
    -Woverloaded-virtual -fstrict-aliasing)

foreach(flag ${test_flags})
  set(test_cxx_flag "CXX_FLAG${flag}")
  CHECK_CXX_COMPILER_FLAG(${flag} "${test_cxx_flag}")
  if (${test_cxx_flag})
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
  endif (${test_cxx_flag})
endforeach(flag ${test_flags})

# memory
check_include_file_cxx ("memory" HAVE_CXX_MEMORY)
# tuple
check_include_file_cxx ("tuple" HAVE_CXX_TUPLE)
