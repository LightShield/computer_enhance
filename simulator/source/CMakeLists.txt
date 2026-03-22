# Add source files to the simulator_main target defined in the parent scope
target_sources(simulator_main
    PRIVATE
        main.cpp
        register_types.cpp
        register_proxy.cpp
        registers.cpp
        simulator.cpp
        commands.cpp
)

# Headers for this component
target_include_directories(simulator_main PRIVATE ../include)
