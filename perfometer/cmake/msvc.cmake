if (LEAN_AND_MEAN)
    add_compile_definitions(_HAS_EXCEPTIONS=0)
    add_compile_options(/EHs-c- /GR-)
endif()
