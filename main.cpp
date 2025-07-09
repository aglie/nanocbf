#include <iostream>
#include <vector>
#include "cbfframe_simple.h"

int main() {
    CbfFrame frame;
    
    // Example: Create a simple 2x2 test image
    std::vector<int32_t> testData = {100, 200, 300, 400};
    std::string testHeader = R"(###CBF: VERSION 1.5
data_test

_array_data.header_convention "PILATUS_1.2"
_array_data.header_contents
;
# Detector: PILATUS 100K, S/N 60-0100
# Pixel_size 172e-6 m x 172e-6 m
# Exposure_time 1.0 s
;

_array_data.data
;
--CIF-BINARY-FORMAT-SECTION--
Content-Type: application/octet-stream;
     conversions="x-CBF_BYTE_OFFSET"
Content-Transfer-Encoding: BINARY
X-Binary-Size: 16
X-Binary-ID: 1
X-Binary-Element-Type: "signed 32-bit integer"
X-Binary-Element-Byte-Order: LITTLE_ENDIAN
Content-MD5: dGVzdA==
X-Binary-Number-of-Elements: 4
X-Binary-Size-Fastest-Dimension: 2
X-Binary-Size-Second-Dimension: 2
X-Binary-Size-Padding: 4095

)";
    
    frame.header = testHeader;
    frame.data = testData;
    frame.width = 2;
    frame.height = 2;
    
    // Write test file
    if (frame.write("test_output.cbf")) {
        std::cout << "Successfully wrote test_output.cbf" << std::endl;
    } else {
        std::cout << "Failed to write CBF file: " << frame.getError() << std::endl;
    }
    
    // Read it back
    CbfFrame readFrame;
    if (readFrame.read("test_output.cbf")) {
        std::cout << "Successfully read CBF file" << std::endl;
        std::cout << "Dimensions: " << readFrame.width << "x" << readFrame.height << std::endl;
        
        std::cout << "Data values: ";
        for (int32_t val : readFrame.data) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    } else {
        std::cout << "Failed to read CBF file: " << readFrame.getError() << std::endl;
    }
    
    return 0;
}
