#!/usr/bin/env bash

# doxygen doxygen.conf
./generate_doc.sh
cd ../html/
git add .
git commit -m "Update documentation"
git push origin gh-pages