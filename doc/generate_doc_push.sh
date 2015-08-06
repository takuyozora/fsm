#!/usr/bin/env bash

doxygen doxygen.conf
cd ../html/
git add .
git commit -m "Update documentation"
git push origin gh-pages