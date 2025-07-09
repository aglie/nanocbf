#include "cbfframe_simple.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <iomanip>
#include <algorithm>
#include <cstring>

// CBF magic number and tail
const std::vector<uint8_t> CbfFrame::CBF_MAGIC = {0x0C, 0x1A, 0x04, 0xD5};
const std::string CbfFrame::CBF_TAIL = std::string(4095, '\0') + "\r\n--CIF-BINARY-FORMAT-SECTION----\r\n;\r\n\r\n";

CbfFrame::CbfFrame() : m_width(0), m_height(0) {}

CbfFrame::~CbfFrame() {}

bool CbfFrame::read(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        m_error = "Could not open file: " + filename;
        return false;
    }
    
    // Read entire file
    std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Find magic number
    auto magicIt = std::search(fileData.begin(), fileData.end(), CBF_MAGIC.begin(), CBF_MAGIC.end());
    if (magicIt == fileData.end()) {
        m_error = "Could not find CBF magic number";
        return false;
    }
    
    // Extract header (everything before magic number)
    size_t headerSize = std::distance(fileData.begin(), magicIt);
    m_header = std::string(fileData.begin(), fileData.begin() + headerSize);
    
    // Parse binary info from header
    int dataSize;
    if (!parseBinaryInfo(m_header, m_width, m_height, dataSize)) {
        return false;
    }
    
    // Extract binary data (after magic number)
    size_t binaryStart = headerSize + CBF_MAGIC.size();
    if (binaryStart + dataSize > fileData.size()) {
        m_error = "File truncated - not enough binary data";
        return false;
    }
    
    std::vector<uint8_t> binaryData(fileData.begin() + binaryStart, fileData.begin() + binaryStart + dataSize);
    
    // Decompress binary data
    m_data = decompressData(binaryData, m_width, m_height);
    
    return true;
}

bool CbfFrame::write(const std::string& filename) const {
    if (m_data.empty() || m_width == 0 || m_height == 0) {
        return false;
    }
    
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Write header
    file.write(m_header.c_str(), m_header.size());
    
    // Write magic number
    file.write(reinterpret_cast<const char*>(CBF_MAGIC.data()), CBF_MAGIC.size());
    
    // Compress and write binary data
    std::vector<uint8_t> compressed = compressData(m_data);
    file.write(reinterpret_cast<const char*>(compressed.data()), compressed.size());
    
    // Write tail
    file.write(CBF_TAIL.c_str(), CBF_TAIL.size());
    
    return true;
}

void CbfFrame::setHeader(const std::string& header) {
    m_header = header;
}

const std::string& CbfFrame::getHeader() const {
    return m_header;
}

void CbfFrame::setData(const std::vector<int32_t>& data, int width, int height) {
    m_data = data;
    m_width = width;
    m_height = height;
}

const std::vector<int32_t>& CbfFrame::getData() const {
    return m_data;
}

bool CbfFrame::parseBinaryInfo(const std::string& header, int& width, int& height, int& dataSize) const {
    std::regex widthRegex(R"(X-Binary-Size-Fastest-Dimension:\s+(\d+))");
    std::regex heightRegex(R"(X-Binary-Size-Second-Dimension:\s+(\d+))");
    std::regex sizeRegex(R"(X-Binary-Size:\s+(\d+))");
    
    std::smatch match;
    
    if (!std::regex_search(header, match, widthRegex)) {
        const_cast<CbfFrame*>(this)->m_error = "Could not find width in header";
        return false;
    }
    width = std::stoi(match[1].str());
    
    if (!std::regex_search(header, match, heightRegex)) {
        const_cast<CbfFrame*>(this)->m_error = "Could not find height in header";
        return false;
    }
    height = std::stoi(match[1].str());
    
    if (!std::regex_search(header, match, sizeRegex)) {
        const_cast<CbfFrame*>(this)->m_error = "Could not find data size in header";
        return false;
    }
    dataSize = std::stoi(match[1].str());
    
    return true;
}

std::vector<uint8_t> CbfFrame::compressData(const std::vector<int32_t>& data) const {
    std::vector<uint8_t> compressed;
    
    int32_t currentValue = 0;
    for (int32_t pixel : data) {
        int32_t delta = pixel - currentValue;
        
        if (delta >= -127 && delta <= 127) {
            // 8-bit delta
            compressed.push_back(static_cast<uint8_t>(delta));
        } else if (delta >= -32767 && delta <= 32767) {
            // 16-bit delta
            compressed.push_back(0x80);
            writeInt16LE(compressed, static_cast<int16_t>(delta));
        } else {
            // 32-bit delta
            compressed.push_back(0x80);
            writeInt16LE(compressed, static_cast<int16_t>(0x8000));
            writeInt32LE(compressed, delta);
        }
        
        currentValue = pixel;
    }
    
    return compressed;
}

std::vector<int32_t> CbfFrame::decompressData(const std::vector<uint8_t>& compressed, int width, int height) const {
    std::vector<int32_t> data;
    data.reserve(width * height);
    
    int32_t currentValue = 0;
    size_t pos = 0;
    
    while (pos < compressed.size() && data.size() < width * height) {
        int8_t delta8 = static_cast<int8_t>(compressed[pos++]);
        
        if (static_cast<uint8_t>(delta8) == 0x80) {
            // 16-bit or 32-bit delta
            if (pos + 1 >= compressed.size()) break;
            
            int16_t delta16 = readInt16LE(&compressed[pos]);
            pos += 2;
            
            if (static_cast<uint16_t>(delta16) == 0x8000) {
                // 32-bit delta
                if (pos + 3 >= compressed.size()) break;
                
                int32_t delta32 = readInt32LE(&compressed[pos]);
                pos += 4;
                
                currentValue += delta32;
            } else {
                currentValue += delta16;
            }
        } else {
            // 8-bit delta
            currentValue += delta8;
        }
        
        data.push_back(currentValue);
    }
    
    return data;
}

void CbfFrame::writeInt16LE(std::vector<uint8_t>& buffer, int16_t value) const {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void CbfFrame::writeInt32LE(std::vector<uint8_t>& buffer, int32_t value) const {
    buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

uint16_t CbfFrame::readInt16LE(const uint8_t* buffer) const {
    return static_cast<uint16_t>(buffer[0]) | (static_cast<uint16_t>(buffer[1]) << 8);
}

uint32_t CbfFrame::readInt32LE(const uint8_t* buffer) const {
    return static_cast<uint32_t>(buffer[0]) | 
           (static_cast<uint32_t>(buffer[1]) << 8) | 
           (static_cast<uint32_t>(buffer[2]) << 16) | 
           (static_cast<uint32_t>(buffer[3]) << 24);
}

std::string CbfFrame::generateMD5(const std::vector<uint8_t>& data) const {
    // Simple MD5 placeholder - in real implementation would use proper MD5 library
    std::ostringstream oss;
    size_t hash = 0;
    for (uint8_t byte : data) {
        hash = hash * 31 + byte;
    }
    oss << std::hex << hash;
    return oss.str();
}