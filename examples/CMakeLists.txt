enable_language(C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

add_executable(vector vector.c)
add_executable(arena arena.c)
add_executable(string string.c)

target_link_libraries(vector misc)
target_link_libraries(arena misc)
target_link_libraries(string misc)

target_link_options(vector PRIVATE -fsanitize=address)
target_link_options(arena PRIVATE -fsanitize=address)
target_link_options(string PRIVATE -fsanitize=address)

add_compile_options(
  -O0
  -g
)

include_directories("../${MISC_INCLUDE_DIR}")
