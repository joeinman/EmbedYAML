#include "EmbedYAML/EmbedYAML.hpp"

namespace EmbedYAML {

EmbedYAML::EmbedYAML(EYFileOpenFunction open, EYFileCloseFunction close, EYReadCharFunction read_char, void* user_context)
    : m_file_open_function(open), m_file_close_function(close), m_read_char_function(read_char), m_user_context(user_context)
{
}

EmbedYAML::~EmbedYAML()
{
}

void EmbedYAML::readFile(std::string filename)
{
    if (m_file_open_function(this, filename) < 0)
    {
        return;
    }

    while (true)
    {
        auto c = m_read_char_function(this);
        if (!c.has_value())
            break;
        printf("%c", c.value());
    }
    printf("\n");
        
    m_file_close_function(this, filename);
}

} // namespace EmbedYAML