set(TYPE_SRC int.cpp double.cpp string.cpp array.cpp) 
add_library(type ${TYPE_SRC})
target_link_libraries(type op)

