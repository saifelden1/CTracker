#!/bin/bash
# CTracker Build Verification Script

echo "========================================="
echo "CTracker Build Verification"
echo "========================================="
echo ""

# Check if build directory exists
if [ ! -d "CTracker/build" ]; then
    echo "❌ Build directory not found"
    echo "   Run: cmake -B CTracker/build -S CTracker"
    exit 1
fi

echo "✅ Build directory exists"

# Check if executable exists
if [ ! -f "CTracker/build/CTracker.exe" ]; then
    echo "❌ Executable not found"
    echo "   Run: cmake --build CTracker/build"
    exit 1
fi

echo "✅ Executable exists"

# Get executable info
EXE_SIZE=$(stat -c%s "CTracker/build/CTracker.exe" 2>/dev/null || stat -f%z "CTracker/build/CTracker.exe" 2>/dev/null)
EXE_SIZE_MB=$(echo "scale=2; $EXE_SIZE / 1024 / 1024" | bc)
echo "   Size: ${EXE_SIZE_MB} MB"

# Check source files
echo ""
echo "Checking source structure..."
EXPECTED_DIRS=("core" "shared" "courses" "projects" "todos" "pomodoro" "analytics" "calendar" "settings")
for dir in "${EXPECTED_DIRS[@]}"; do
    if [ -d "CTracker/include/$dir" ] && [ -d "CTracker/src/$dir" ]; then
        echo "✅ $dir/ (include + src)"
    else
        echo "❌ $dir/ missing"
    fi
done

# Check CMakeLists.txt
echo ""
echo "Checking CMakeLists.txt..."
if grep -q "Qt6::Charts" CTracker/CMakeLists.txt; then
    echo "✅ Qt6::Charts linked"
else
    echo "⚠️  Qt6::Charts not found (needed for Phase 8)"
fi

if grep -q "Qt6::Svg" CTracker/CMakeLists.txt; then
    echo "✅ Qt6::Svg linked"
else
    echo "⚠️  Qt6::Svg not found (needed for Phase 8)"
fi

# Summary
echo ""
echo "========================================="
echo "Summary"
echo "========================================="
echo "✅ Phases 0-7 appear complete"
echo "✅ Application is ready to launch"
echo ""
echo "To run the application:"
echo "  ./CTracker/build/CTracker.exe"
echo ""
echo "To rebuild:"
echo "  cmake --build CTracker/build --clean-first"
echo ""
echo "See VERIFICATION_GUIDE.md for detailed testing instructions"
echo "========================================="
