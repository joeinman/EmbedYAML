add_executable(EmbedLogParsingExample
    "Parsing.cpp"
)

target_link_libraries(EmbedLogParsingExample
    PUBLIC
        EmbedYAML
)

add_executable(EmbedLogEmittingExample
    "Emitting.cpp"
)

target_link_libraries(EmbedLogEmittingExample
    PUBLIC
        EmbedYAML
)
