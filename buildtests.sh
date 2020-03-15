rm -rf build
mkdir build
cp test_inputs/* build/
for file in $(ls tests)
do
    echo "Building $file..."
    gcc -o build/$file.out tests/$file src/quicklight.c src/qlrender.c src/qslt.c -g -lm -lX11
    echo "Built."
done