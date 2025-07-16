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
)

target_compile_definitions(flecs PUBLIC ${FLECS_DEFINES})
target_compile_definitions(flecs_static PUBLIC ${FLECS_DEFINES})