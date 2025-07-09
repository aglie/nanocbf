# nanocbf

A minimal, fast C++ library for reading and writing CBF (Crystallographic Binary Format) files.

## Features

- **Simple API**: Just 4 public fields and 2 methods
- **No dependencies**: Pure C++17, no external libraries required
- **Fast**: SIMD-optimized compression/decompression
- **Compatible**: Works with CBF files from XDS, PILATUS, and other generators
- **Standards compliant**: Generates proper MD5 checksums and CBF format

## Quick Start

```cpp
#include "cbfframe_simple.h"

// Write CBF file
CbfFrame frame;
frame.data = {100, 200, 300, 400};  // 1D vector of pixel values
frame.width = 2;
frame.height = 2;
frame.header = ""; // Empty = use default header
frame.write("output.cbf");

// Read CBF file
CbfFrame readFrame;
readFrame.read("input.cbf");
std::cout << "Size: " << readFrame.width << "x" << readFrame.height << std::endl;
```

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Complete Example

```cpp
#include <iostream>
#include <vector>
#include "cbfframe_simple.h"

int main() {
    // ===== WRITING CBF FILES =====
    
    // Create a simple 3x3 detector image
    std::vector<int32_t> imageData = {
        10,  20,  30,
        40,  50,  60, 
        70,  80,  90
    };
    
    CbfFrame frame;
    frame.data = imageData;
    frame.width = 3;
    frame.height = 3;
    
    // Option 1: Use default header (minimal)
    frame.header = "";
    if (frame.write("simple_output.cbf")) {
        std::cout << "✓ Wrote simple CBF file" << std::endl;
    }
    
    // Option 2: Use custom header with detector info
    frame.header = R"(_array_data.header_convention "PILATUS_1.2"
_array_data.header_contents
;
# Detector: PILATUS 100K, S/N 60-0100
# 2024-07-09T15:30:00.000
# Pixel_size 172e-6 m x 172e-6 m
# Silicon sensor, thickness 0.000320 m
# Exposure_time 1.0000 s
# Exposure_period 1.0000 s
# Tau = 124.0e-09 s
# Count_cutoff 388705 counts
# Threshold_setting: 8000 eV
# Gain_setting: low gain (vrf = -0.300)
# Wavelength 1.54180 A
# Energy_range (0, 0) eV
# Detector_distance 0.100 m
# Detector_Voffset 0.00000 m
# Beam_xy (1.50, 1.50) pixels
# Flux 1000000.0
# Filter_transmission 100.0000
# Start_angle 0.0000 deg.
# Angle_increment 0.1000 deg.
# Detector_2theta 0.0000 deg.
# Polarization 0.990
# Alpha 0.0000 deg.
# Kappa 0.0000 deg.
# Phi 0.0000 deg.
# Phi_increment 0.1000 deg.
# Omega 0.0000 deg.
# Omega_increment 0.0000 deg.
# Chi 0.0000 deg.
# Chi_increment 0.0000 deg.
# Oscillation_axis PHI
# N_oscillations 1
;

)";
    
    if (frame.write("detailed_output.cbf")) {
        std::cout << "✓ Wrote detailed CBF file" << std::endl;
    }
    
    // ===== READING CBF FILES =====
    
    // Read the file we just created
    CbfFrame readFrame;
    if (readFrame.read("detailed_output.cbf")) {
        std::cout << "✓ Successfully read CBF file" << std::endl;
        
        // Print basic info
        std::cout << "Dimensions: " << readFrame.width << "x" << readFrame.height << std::endl;
        std::cout << "Total pixels: " << readFrame.data.size() << std::endl;
        
        // Print first few pixel values
        std::cout << "First 5 pixel values: ";
        for (int i = 0; i < std::min(5, (int)readFrame.data.size()); i++) {
            std::cout << readFrame.data[i] << " ";
        }
        std::cout << std::endl;
        
        // Print extracted header
        std::cout << "Extracted header:" << std::endl;
        std::cout << readFrame.header << std::endl;
        
        // Process the data - example: calculate statistics
        int32_t minVal = *std::min_element(readFrame.data.begin(), readFrame.data.end());
        int32_t maxVal = *std::max_element(readFrame.data.begin(), readFrame.data.end());
        int64_t sum = 0;
        for (int32_t val : readFrame.data) {
            sum += val;
        }
        double mean = (double)sum / readFrame.data.size();
        
        std::cout << "Statistics:" << std::endl;
        std::cout << "  Min: " << minVal << std::endl;
        std::cout << "  Max: " << maxVal << std::endl;
        std::cout << "  Mean: " << mean << std::endl;
        
    } else {
        std::cout << "✗ Failed to read CBF file: " << readFrame.getError() << std::endl;
    }
    
    // ===== READING EXTERNAL CBF FILES =====
    
    // Example: Read a CBF file from XDS or other software
    CbfFrame externalFrame;
    if (externalFrame.read("diffraction_001.cbf")) {
        std::cout << "✓ Read external CBF file" << std::endl;
        std::cout << "Detector size: " << externalFrame.width << "x" << externalFrame.height << std::endl;
        
        // Find hot pixels (example threshold: > 10000 counts)
        int hotPixels = 0;
        for (int32_t pixel : externalFrame.data) {
            if (pixel > 10000) hotPixels++;
        }
        std::cout << "Hot pixels (>10000 counts): " << hotPixels << std::endl;
        
        // Save processed version
        // Example: Apply background subtraction
        for (int32_t& pixel : externalFrame.data) {
            pixel = std::max(0, pixel - 100);  // Subtract background
        }
        
        if (externalFrame.write("processed_diffraction_001.cbf")) {
            std::cout << "✓ Saved processed CBF file" << std::endl;
        }
    } else {
        std::cout << "ℹ No external CBF file found (this is normal)" << std::endl;
    }
    
    // ===== BATCH PROCESSING EXAMPLE =====
    
    std::vector<std::string> cbfFiles = {
        "frame_001.cbf", "frame_002.cbf", "frame_003.cbf"
    };
    
    std::cout << "Processing batch of CBF files..." << std::endl;
    for (const auto& filename : cbfFiles) {
        CbfFrame batchFrame;
        if (batchFrame.read(filename)) {
            // Process each frame
            int64_t totalCounts = 0;
            for (int32_t pixel : batchFrame.data) {
                totalCounts += pixel;
            }
            std::cout << filename << ": " << totalCounts << " total counts" << std::endl;
        } else {
            std::cout << filename << ": File not found (expected)" << std::endl;
        }
    }
    
    return 0;
}
```

## API Reference

### CbfFrame Class

**Public Fields:**
- `std::string header` - Text header content (leave empty for default)
- `std::vector<int32_t> data` - 1D vector of pixel values
- `int width` - Image width in pixels
- `int height` - Image height in pixels

**Methods:**
- `bool read(const std::string& filename)` - Read CBF file
- `bool write(const std::string& filename)` - Write CBF file
- `const std::string& getError()` - Get last error message

### Automatic Features

- **Header generation**: CBF version line and data section name from filename
- **MD5 checksums**: Automatically calculated for data integrity
- **Compression**: CBF_BYTE_OFFSET compression for efficient storage
- **Format compliance**: Proper CRLF line endings and CBF structure

## Format Support

**Reading:** Works with CBF files from:
- XDS (X-ray Detector Software)
- PILATUS detectors
- Other CBF-compliant software
- Files with or without MD5 checksums

**Writing:** Generates standard-compliant CBF files with:
- Proper binary section headers
- MD5 checksums
- CBF_BYTE_OFFSET compression
- CRLF line endings

## License

This is a minimal implementation focused on simplicity and performance.