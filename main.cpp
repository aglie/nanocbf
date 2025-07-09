#include <iostream>
#include <vector>
#include "cbfframe_simple.h"

int main() {
    CbfFrame frame;
    
    // Example: Create a simple 2x2 test image
    std::vector<int32_t> testData = {100, 200, 300, 400};
    
    // Test with empty header - should use default
    frame.data = testData;
    frame.width = 2;
    frame.height = 2;
    
    // Write test file
    if (frame.write("test_output.cbf")) {
        std::cout << "Successfully wrote test_output.cbf with default header" << std::endl;
    } else {
        std::cout << "Failed to write CBF file: " << frame.getError() << std::endl;
    }
    
    // Test with custom header
    CbfFrame frame2;
    std::string customHeader = R"(_array_data.header_convention "PILATUS_1.2"
_array_data.header_contents
;
# Detector: PILATUS 100K, S/N 60-0100
# Pixel_size 172e-6 m x 172e-6 m
# Exposure_time 1.0 s
;

)";
    
    frame2.header = customHeader;
    frame2.data = testData;
    frame2.width = 2;
    frame2.height = 2;
    
    if (frame2.write("test_custom.cbf")) {
        std::cout << "Successfully wrote test_custom.cbf with custom header" << std::endl;
    } else {
        std::cout << "Failed to write custom CBF file: " << frame2.getError() << std::endl;
    }
    
    // Read back the default header version
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

    // Read back the default header version
    CbfFrame frame3;
    if (frame3.read("../test_data/Y-CORRECTIONS.cbf")) {
        std::cout << "Successfully read CBF file from XDS" << std::endl;
        std::cout << "Dimensions: " << frame3.width << "x" << frame3.height << std::endl;
    } else {
        std::cout << "Failed to read CBF file: " << readFrame.getError() << std::endl;
    }
    
    return 0;
}
