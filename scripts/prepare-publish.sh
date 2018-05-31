#!/bin/sh

cd "$(dirname "$0")"/..

rm -rf _stage
mkdir -p _stage
cd _stage
git clone .. libdvbtee_parser
cd libdvbtee_parser

node scripts/prepare-build.js
node scripts/configure-build.js

sed -i s/"npm install git-clone mkdirp rimraf && node scripts\/prepare-build.js && node scripts\/configure-build.js"/""/1 package.json
sed -i s/"^libdvbpsi\/"/""/1 .gitignore
sed -i s/"^usr\/"/""/1 .gitignore
sed -i s/"^config.h"/""/1 .gitignore
cd libdvbpsi
sed -i s/"^config.h"/""/1 .gitignore
cd ..
cd libdvbtee
sed -i s/"static bool hello = false"/"static bool hello = true"/1 parse.cpp
cd ..

npm publish

cd ../..
