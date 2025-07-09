#ifndef CBFFRAME_SIMPLE_H
#define CBFFRAME_SIMPLE_H

#include <vector>
#include <string>
#include <cstdint>

class CbfFrame {
public:
    CbfFrame();
    ~CbfFrame();
    
    // Read CBF file
    bool read(const std::string& filename);
    
    // Write CBF file
    bool write(const std::string& filename) const;
    
    // Set text header (will be written before binary data)
    void setHeader(const std::string& header);
    
    // Get text header
    const std::string& getHeader() const;
    
    // Set binary data
    void setData(const std::vector<int32_t>& data, int width, int height);
    
    // Get binary data
    const std::vector<int32_t>& getData() const;
    
    // Get dimensions
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    // Get error message
    const std::string& getError() const { return m_error; }
    
private:
    static const std::vector<uint8_t> CBF_MAGIC;
    static const std::string CBF_TAIL;
    
    std::string m_header;
    std::vector<int32_t> m_data;
    int m_width;
    int m_height;
    std::string m_error;
    
    // Binary data compression/decompression
    std::vector<uint8_t> compressData(const std::vector<int32_t>& data) const;
    std::vector<int32_t> decompressData(const std::vector<uint8_t>& compressed, int width, int height) const;
    
    // Utility functions
    void writeInt16LE(std::vector<uint8_t>& buffer, int16_t value) const;
    void writeInt32LE(std::vector<uint8_t>& buffer, int32_t value) const;
    uint16_t readInt16LE(const uint8_t* buffer) const;
    uint32_t readInt32LE(const uint8_t* buffer) const;
    
    // Parse binary header info from text header
    bool parseBinaryInfo(const std::string& header, int& width, int& height, int& dataSize) const;
    
    // Generate MD5 hash
    std::string generateMD5(const std::vector<uint8_t>& data) const;
};

#endif // CBFFRAME_SIMPLE_H