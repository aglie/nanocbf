## Project Overview

This is **minicbf**, a C++ library for reading and writing CBF (Crystallographic Binary Format) files used in X-ray crystallography. The library is designed for high-performance processing of detector data from scientific instruments like PILATUS detectors used in synchrotron experiments.

## Build Commands

```bash
# Build the project
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake ..
make

# Run the executable (currently just prints "Hello, World!")
./nanocbf
```

**Build Dependencies:**
- CMake 3.25+
- C++17 compiler

## Architecture Overview

The codebase follows a template-based object-oriented design with three main layers:

### 1. Template Framework (`frame.h`)
- **`Frame<T>`**: Generic 2D array container with mathematical operations
- Provides rotation, flipping, statistical functions, and arithmetic operations
- Template design supports multiple data types (double, int32_t, etc.)

### 2. Crystallographic Framework (`cryioframe.h/cpp`)
- **`CryioFrame`**: Abstract base class extending `Frame<double>`
- Handles crystallographic metadata (goniometer angles, detector parameters)
- Manages experimental conditions (exposure time, wavelength, beam center)
- Defines interface for file I/O operations

### 3. CBF Format Implementation (`cbfframe.h/cpp`)
- **`CbfHeader`**: Parses CBF text headers using regex patterns
- **`CbfFrame`**: Complete CBF file handling with binary data compression
- Supports PILATUS detector format with differential encoding
- Handles MD5 checksum validation and metadata extraction

### 4. Performance Library (`llr/`)
SIMD-optimized mathematical operations with multiple implementation levels:
- **`sumroi.h/c`**: Region of interest summation (AVX/SSE4/AVX512)
- **`sumarr.h/c`**: Array summation with vectorized implementations
- **`setarrval.h/c`**: Array value setting with SIMD support
- **`avx.h/c`**: AVX capability detection and intrinsics

## Key Classes and Relationships

```
Frame<T> (template base)
    ↓
CryioFrame : Frame<double> (crystallographic abstraction)
    ↓
CbfHeader : CryioFrame (CBF header parsing)
    ↓
CbfFrame : CbfHeader (full CBF with binary data)
```

## CBF File Format Details

CBF files contain:
1. **Text header** with metadata and experimental parameters
2. **Binary data** using CBF_BYTE_OFFSET compression

**Key binary format markers:**
- Magic number: `\x0c\x1a\x04\xd5` (separates header from binary data)
- Binary section delimiter: `--CIF-BINARY-FORMAT-SECTION--`
- Compression uses differential encoding with variable-length integers

**Binary metadata extracted from header:**
- `X-Binary-Size`: Compressed data size
- `X-Binary-Number-of-Elements`: Total pixels
- `X-Binary-Size-Fastest-Dimension`: Width
- `X-Binary-Size-Second-Dimension`: Height
- `Content-MD5`: Data integrity checksum

## Development Notes

### Current Status
- Library is fully functional for CBF reading/writing
- Main executable (`main.cpp`) is a placeholder that prints "Hello, World!"
- Build system is minimal but functional
- No existing tests or linting configuration

### Python Integration
The `psi_cbf/` directory contains a Python package from Paul Scherrer Institute:
- Provides Python bindings for CBF functionality
- Integrates with numpy arrays
- Includes C extension for performance-critical operations

### Performance Optimizations
The LLR library provides multiple SIMD implementations:
- **FPU**: Basic floating-point operations
- **SSE4.1**: 128-bit SIMD
- **AVX2**: 256-bit SIMD
- **AVX512**: 512-bit SIMD

Runtime detection selects optimal implementation based on CPU capabilities.

### Template Design Patterns
- Heavy use of templates for type flexibility
- SFINAE and template specialization for performance
- Template functions for mathematical operations across different numeric types

## Common Development Patterns

### Reading CBF Files
```cpp
CbfFrame frame("example.cbf");
if (frame.read()) {
    QVector<double>& data = frame.array();
    // Process data...
}
```

### Accessing Metadata
```cpp
QString exposureTime = frame["Exposure_time"];
QString wavelength = frame["Wavelength"];
double beamX = frame["Beam_x"].toDouble();
```

### Mathematical Operations
```cpp
// SIMD-optimized operations
double totalSum = sum_array_avx(data.data(), data.size());
set_array_value_avx(data.data(), data.size(), 0.0); // Zero array
```

## Scientific Context

This library is designed for:
- X-ray crystallography data processing
- Synchrotron beamline data analysis
- High-performance scientific computing
- Integration with crystallographic software pipelines

The focus on performance and standards compliance makes it suitable for processing large volumes of detector data in research environments.