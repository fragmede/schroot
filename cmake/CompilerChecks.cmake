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
