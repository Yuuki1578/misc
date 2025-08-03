enable_language(C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

add_executable(vector vector.c)
add_executable(arena arena.c)
add_executable(string string.c)
add_executable(refcount refcount.c)

target_link_libraries(vector misc)
target_link_libraries(arena misc)
target_link_libraries(string misc)
target_link_libraries(refcount misc)

if (NOT MSVC)
  target_link_options(vector PRIVATE -fsanitize=address)
  target_link_options(arena PRIVATE -fsanitize=address)
  target_link_options(string PRIVATE -fsanitize=address)
  target_link_options(refcount PRIVATE -fsanitize=address)

  add_compile_options(
    -O0
    -g
  )
endif ()

include_directories("../${MISC_INCLUDE_DIR}")
