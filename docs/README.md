# campello_input Documentation

This directory contains documentation source files for the campello_input library.

## Building Documentation

Documentation is generated using [Doxygen](https://www.doxygen.nl/).

### Prerequisites

```bash
# macOS
brew install doxygen

# Ubuntu/Debian
sudo apt-get install doxygen

# Windows (via chocolatey)
choco install doxygen
```

### Generate Documentation

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTS=OFF
cmake --build . --target docs
```

The generated HTML documentation will be in `build/docs/html/`.
Open `build/docs/html/index.html` in your browser.

### CMake Integration

When Doxygen is found during CMake configuration, a `docs` target is automatically created:

```bash
cmake --build . --target docs
```

Documentation is also installed with the library:

```bash
cmake --install . --prefix /usr/local
# Documentation will be in /usr/local/share/doc/campello_input/
```

## Documentation Structure

```
docs/
├── Doxyfile.in          # Doxygen configuration template
├── README.md            # This file
└── pages/               # Markdown documentation pages
    ├── mainpage.md      # Main landing page
    └── usage_guide.md   # Detailed usage guide
```

## API Documentation

The API documentation is generated from:

1. **Header files** (`inc/campello_input/*.hpp`) - Inline documentation
2. **Markdown pages** (`docs/pages/*.md`) - Guides and tutorials
3. **Examples** (`examples/`) - Code examples

## Adding Documentation

### Documenting Code

Use Doxygen-style comments in header files:

```cpp
/**
 * @brief Brief description
 * @param paramName Parameter description
 * @return Return value description
 * @see RelatedFunction
 */
[[nodiscard]] bool getState(GamepadState& outState) const;
```

### Adding Pages

1. Create a new `.md` file in `docs/pages/`
2. Add a page reference in `Doxyfile.in` INPUT section
3. Link from other pages using `@ref page_name`

## Hosting Documentation

### GitHub Pages

To host on GitHub Pages:

```bash
# Generate docs
cmake --build build --target docs

# Copy to docs/ folder for GitHub Pages
cp -r build/docs/html/* docs/

# Commit and push
git add docs/
git commit -m "Update documentation"
git push
```

Enable GitHub Pages in repository settings to serve from `/docs` folder.

### Local Server

```bash
cd build/docs/html
python3 -m http.server 8000
# Open http://localhost:8000 in browser
```
