px4_add_module(
    MODULE examples__helloworld
    MAIN helloworld
    STACK_MAIN 2000
    SRCS
        helloworld.c
    DEPENDS
        platforms__common
    )
# vim: set noet ft=cmake fenc=utf-8 ff=unix : 