SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Generic)

# which compilers to use for C and C++
SET(CMAKE_C_FLAGS --specs=nosys.specs)
SET(CMAKE_C_COMPILER arm-none-eabi-gcc)
SET(CMAKE_CXX_COMPILER arm-none-eabi-g++)
SET(CMAKE_CXX_COMPILER_WORKS 1)
SET(CMAKE_SIZE arm-none-eabi-size)
SET(CMAKE_OBJCOPY arm-none-eabi-objcopy)
SET(CMAKE_OBJDUMP arm-none-eabi-objdump)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
