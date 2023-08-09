#!/bin/bash

# This script can be used to format all the C++ code in the project. It can also be
# used to check if the code is formatted correctly. The script will be used by the CI.
# If you want to check if the code is formatted correctly, run the script with --check.
# This script should be run from the root of the project.

if [ "$1" == "--check" ]; then
    find examples/ include/ test/ tooling/ -iname *.cpp -o -iname *.hpp -o -iname *.h -o -iname *.inl | xargs clang-format-15 -i -style=file --dry-run --Werror
else 
    find examples/ include/ test/ tooling/ -iname *.cpp -o -iname *.hpp -o -iname *.h -o -iname *.inl | xargs clang-format -i -style=file
fi

