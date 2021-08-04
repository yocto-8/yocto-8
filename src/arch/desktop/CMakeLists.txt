add_executable(y8vm
    hal/hal.cpp
    main.cpp
    window.cpp
)

set_property(TARGET y8vm PROPERTY INTERPROCEDURAL_OPTIMIZATION ON)
target_link_libraries(y8vm yocto-8-core sfml-system sfml-window sfml-graphics)