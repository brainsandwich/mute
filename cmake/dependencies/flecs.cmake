include(CPM)

CPMAddPackage("https://github.com/SanderMertens/flecs.git@4.0.3")

set(FLECS_DEFINES
# target_compile_definitions(flecs PUBLIC
    FLECS_CUSTOM_BUILD
        FLECS_CPP
        # FLECS_MODULE
        # FLECS_SCRIPT
        # FLECS_STATS
        # FLECS_METRICS
        # FLECS_ALERTS
        FLECS_SYSTEM
        FLECS_PIPELINE
        FLECS_TIMER
        # FLECS_META
        # FLECS_UNITS
        # FLECS_JSON
        # FLECS_DOC
        FLECS_LOG
        # FLECS_APP
        FLECS_OS_API_IMPL
        # FLECS_HTTP
        # FLECS_REST
        # FLECS_JOURNAL
        # FLECS_PERF_TRACE

    # # FLECS_CUSTOM_BUILD
    #     # FLECS_CPP
    #     FLECS_NO_MODULE
    #     FLECS_NO_SCRIPT
    #     FLECS_NO_STATS
    #     FLECS_NO_METRICS
    #     # FLECS_ALERTS
    #     # FLECS_SYSTEM
    #     # FLECS_PIPELINE
    #     # FLECS_TIMER
    #     FLECS_NO_META
    #     FLECS_NO_UNITS
    #     FLECS_NO_JSON
    #     FLECS_NO_DOC
    #     # FLECS_LOG
    #     FLECS_NO_APP
    #     # FLECS_OS_API_IMPL
    #     FLECS_NO_HTTP
    #     FLECS_NO_REST
    #     FLECS_NO_JOURNAL
    #     FLECS_NO_PERF_TRACE
)

target_compile_definitions(flecs PUBLIC ${FLECS_DEFINES})
target_compile_definitions(flecs_static PUBLIC ${FLECS_DEFINES})