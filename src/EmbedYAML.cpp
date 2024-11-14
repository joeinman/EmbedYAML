#include "EmbedYAML/EmbedYAML.hpp"

#define STRVAL(x) ((x) ? (char*)(x) : "")

void indent(int level)
{
    int i;
    for (i = 0; i < level; i++) {
        printf("%s", "  ");
    }
}

void print_event(yaml_event_t *event)
{
    static int level = 0;

    switch (event->type) {
    case YAML_NO_EVENT:
        indent(level);
        printf("no-event (%d)\n", event->type);
        break;
    case YAML_STREAM_START_EVENT:
        indent(level++);
        printf("stream-start-event (%d)\n", event->type);
        break;
    case YAML_STREAM_END_EVENT:
        indent(--level);
        printf("stream-end-event (%d)\n", event->type);
        break;
    case YAML_DOCUMENT_START_EVENT:
        indent(level++);
        printf("document-start-event (%d)\n", event->type);
        break;
    case YAML_DOCUMENT_END_EVENT:
        indent(--level);
        printf("document-end-event (%d)\n", event->type);
        break;
    case YAML_ALIAS_EVENT:
        indent(level);
        printf("alias-event (%d)\n", event->type);
        break;
    case YAML_SCALAR_EVENT:
        indent(level);
        printf("scalar-event (%d) = {value=\"%s\", length=%d}\n",
               event->type,
               STRVAL(event->data.scalar.value),
               (int)event->data.scalar.length);
        break;
    case YAML_SEQUENCE_START_EVENT:
        indent(level++);
        printf("sequence-start-event (%d)\n", event->type);
        break;
    case YAML_SEQUENCE_END_EVENT:
        indent(--level);
        printf("sequence-end-event (%d)\n", event->type);
        break;
    case YAML_MAPPING_START_EVENT:
        indent(level++);
        printf("mapping-start-event (%d)\n", event->type);
        break;
    case YAML_MAPPING_END_EVENT:
        indent(--level);
        printf("mapping-end-event (%d)\n", event->type);
        break;
    }
    if (level < 0) {
        fprintf(stderr, "indentation underflow!\n");
        level = 0;
    }
}

namespace EmbedYAML {

EmbedYAML::EmbedYAML(EYFileOpenFunction open,
                     EYFileCloseFunction close,
                     EYReadCharFunction read_char,
                     void* user_context)
    : m_file_open_function(open),
      m_file_close_function(close),
      m_read_char_function(read_char),
      m_user_context(user_context)
{
}

EmbedYAML::~EmbedYAML()
{
}

bool EmbedYAML::parseFile(std::string filename)
{
    if (m_file_open_function(this, filename) < 0)
        return false;

        yaml_parser_t parser;
    yaml_parser_initialize(&parser);

    yaml_parser_set_input(
        &parser,
        [](void* ext, unsigned char* buffer, size_t size, size_t* length) -> int
        {
            auto ey = (EmbedYAML*)ext;
            *length = 0;

            for (size_t i = 0; i < size; ++i) {
                auto c = ey->m_read_char_function(ey);
                if (!c.has_value()) {
                    // Return success with fewer bytes read if we're at the end
                    return 1; 
                }
                buffer[i] = c.value();
                (*length)++;
            }

            return 1;
        },
        this
    );

    bool done = false;
    while(!done)
    {
        yaml_event_t event;
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "parser error %d\n", parser.error);
            break;
        }

        print_event(&event);

        done = (event.type == YAML_STREAM_END_EVENT);
        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);

    if (m_file_close_function(this, filename) < 0)
        return false;

    return true;
}

} // namespace EmbedYAML
