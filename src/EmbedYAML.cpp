#include "EmbedYAML/EmbedYAML.hpp"

namespace EmbedYAML {

void print_event(yaml_event_t event);

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

YAMLNode EmbedYAML::parseFile(std::string filename)
{
    YAMLNode root("root");
    std::stack<YAMLNode*> node_stack;
    node_stack.push(&root);

    if (m_file_open_function(this, filename) < 0)
        return root;

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

    bool mapping_started = true;
    std::string key = "";
    std::string value = "";

    bool done = false;
    bool whole_map_started = false;
    while(!done)
    {
        yaml_event_t event;
        if (!yaml_parser_parse(&parser, &event)) {
            fprintf(stderr, "parser error %d\n", parser.error);
            break;
        }

        print_event(event);

        switch (event.type)
        {
        case YAML_SCALAR_EVENT:
            {
                if (mapping_started) {
                    node_stack.top()->addSequence((char*)event.data.scalar.value, std::vector<YAMLNode>());
                    node_stack.push(&node_stack.top()->operator[]((char*)event.data.scalar.value));
                    std::cout << "Map: " << (char*)event.data.scalar.value << std::endl;
                    mapping_started = false;
                } else {
                    if (key.empty()) {
                        key = (char*)event.data.scalar.value;
                    } else {
                        value = (char*)event.data.scalar.value;
                        node_stack.top()->addScalar(key, value);
                        std::cout << "Scalar: " << key << " = " << value << std::endl;
                        key = "";
                        value = "";
                    }
                }
            }
            break;
        case YAML_MAPPING_START_EVENT:
        case YAML_SEQUENCE_START_EVENT:
            {
                // if(whole_map_started)
                //     mapping_started = true;
                // else
                //     whole_map_started = true;
            }
            break;
        case YAML_MAPPING_END_EVENT:
        case YAML_SEQUENCE_END_EVENT:
            {
                node_stack.pop();
            }
            break;
        default:
            break;
        }

        done = (event.type == YAML_STREAM_END_EVENT);
        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);

    if (m_file_close_function(this, filename) < 0)
        return root;

    return root;
}

void print_event(yaml_event_t event)
{
    std::cout << "Event type: ";
    switch (event.type)
    {
    case YAML_NO_EVENT:
        std::cout << "No event" << std::endl;
        break;
    case YAML_STREAM_START_EVENT:
        std::cout << "Stream start" << std::endl;
        break;
    case YAML_STREAM_END_EVENT:
        std::cout << "Stream end" << std::endl;
        break;
    case YAML_DOCUMENT_START_EVENT:
        std::cout << "Document start" << std::endl;
        break;
    case YAML_DOCUMENT_END_EVENT:
        std::cout << "Document end" << std::endl;
        break;
    case YAML_ALIAS_EVENT:
        std::cout << "Alias" << std::endl;
        break;
    case YAML_SCALAR_EVENT:
        std::cout << "Scalar" << std::endl;
        break;
    case YAML_SEQUENCE_START_EVENT:
        std::cout << "Sequence start" << std::endl;
        break;
    case YAML_SEQUENCE_END_EVENT:
        std::cout << "Sequence end" << std::endl;
        break;
    case YAML_MAPPING_START_EVENT:
        std::cout << "Mapping start" << std::endl;
        break;
    case YAML_MAPPING_END_EVENT:
        std::cout << "Mapping end" << std::endl;
        break;
    default:
        break;
    }
}

} // namespace EmbedYAML
