
find_package(Python3 REQUIRED COMPONENTS  Interpreter Development NumPy)
include_directories(SYSTEM
    ${Python3_INCLUDE_DIRS} 
    ${Python3_NumPy_INCLUDE_DIRS}
    )
    
add_subdirectory(base)
add_subdirectory(dataformat)
add_subdirectory(processor)
add_subdirectory(pyutil)
add_subdirectory(cpputil)
