#ifndef CSVRow_DEFINED
#define CSVRow_DEFINED

#include <string>
#include <vector>
#include <iostream>

// https://stackoverflow.com/a/46931770/13680015

#include <sstream>
std::vector<std::string> split (const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}


class CSVRow
{
    private:
        std::string                 m_line;
        std::vector<std::string>    m_data;

    public:
        std::string operator[](std::size_t index) const
        {
            return m_data[index];
        }
        std::size_t size() const
        {
            return m_data.size();
        }
        std::istream& readNextRow(std::istream& is)
        {
            std::getline(is, m_line);
            m_data = split(m_line, ',');
            return is;
        }
};

std::istream& operator>>(std::istream& is, CSVRow& data)
{
    return data.readNextRow(is);
}

#endif // CSVRow_DEFINED
