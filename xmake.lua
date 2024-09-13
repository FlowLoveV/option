add_rules("mode.debug", "mode.release")

target("test_option")
    set_kind("binary")
    set_languages("c++23")
    add_includedirs("$(projectdir)")
    add_packages("cpptrace")
    add_files("test.cpp")
target_end()
