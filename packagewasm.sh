#!/bin/sh

rm -rf wasm-latest
mkdir wasm-latest


cp build-labeller-Qt_5_15_0_WebAssembly-Release/*.js wasm-latest
cp build-labeller-Qt_5_15_0_WebAssembly-Release/*.wasm wasm-latest
cp build-labeller-Qt_5_15_0_WebAssembly-Release/*.html wasm-latest
cp build-labeller-Qt_5_15_0_WebAssembly-Release/*.svg wasm-latest

cd wasm-latest
zip -r ../labeller-wasm.zip * 
cd ..


