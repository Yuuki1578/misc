enable_testing()

set(MISC_FEATURES_ARENA "arena")
set(MISC_FEATURES_VECTOR "vector")
set(MISC_FEATURES_TCP "tcp")
set(MISC_FEATURES_TCP_CLIENT "tcp_client")
set(MISC_FEATURES_MMAP "mmap")
set(MISC_FEATURES_FILE "file")

include_directories("../include")

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
  -O3
  -march=native
  -mtune=native
  -ffast-math
  -funroll-loops
  -fomit-frame-pointer
  -Wall
  -Werror
  -Wextra
  -Wno-pointer-arith
  -pedantic
)

add_link_options(
  -fsanitize=address
)

add_executable(${MISC_FEATURES_ARENA} arena.c)
target_link_libraries(${MISC_FEATURES_ARENA} ${PROJECT_NAME})

add_executable(${MISC_FEATURES_VECTOR} vector.c)
target_link_libraries(${MISC_FEATURES_VECTOR} ${PROJECT_NAME})

add_executable(${MISC_FEATURES_TCP} tcp.c)
target_link_libraries(${MISC_FEATURES_TCP} ${PROJECT_NAME})

add_executable(${MISC_FEATURES_TCP_CLIENT} tcp_client.c)
target_link_libraries(${MISC_FEATURES_TCP_CLIENT} ${PROJECT_NAME})

add_executable(${MISC_FEATURES_MMAP} mmap.c)
target_link_libraries(${MISC_FEATURES_MMAP} ${PROJECT_NAME})

add_executable(${MISC_FEATURES_FILE} file.c)
target_link_libraries(${MISC_FEATURES_FILE} ${PROJECT_NAME})
