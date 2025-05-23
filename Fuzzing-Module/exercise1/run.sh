# This file contains code we tried to implement: allowlist, control flow graphs, decision trees, etc.
# After some more research, we realised that we don't need to implement all of this, but rather focus on the afl++ instrumentation.
# However, this code can still be usufel for future reference and to understand a little bit how cppcheck works and how to extract function names from source code, or functions names from the llvm instrumentation.

#!/bin/bash
# run these lines once before you first run this code  to install the necessary libraries:
#apt update && apt install build-essential cmake unzip curl llvm clang llvm-dev clang-tools universal-ctags python3-full cppcheck -y
#python3 -m pip install --upgrade pip setuptools wheel
#python3 -m pip install llvmlite
#apt install graphviz -y

# Determine the script directory
script_dir="$(dirname "$0")"

allowlist="$script_dir/allowlist.txt"
python3 problematic-files.py > "$allowlist"
sort -u "$allowlist" -o "$allowlist"

control_flow_graph="$script_dir/control_flow_graph.txt"
python3 decision_tree.py > "$control_flow_graph"
#
decision_tree="$script_dir/decision_tree.txt"
python3 dec_tree_1.py > "$decision_tree"

 echo "Cppcheck done!"

echo "Cppcheck found issues in the following functions"
cat "$allowlist"

 echo "cfg  is"
 cat "$control_flow_graph"
#
echo "decision tree is"
cat "$decision_tree"

# Add the file with problematic functions to AFL_LLVM_ALLOWLIST (environment variable)
 echo "Compiling with AFL++ instrumentation..."
 if [ -n "$allowlist" ]; then
     export AFL_LLVM_ALLOWLIST="$allowlist"
 fi

#mkdir -p "$script_dir/exercise1"
#afl_result="$script_dir/exercise1/afl_result.txt"


#cd "$script_dir/.."
# /AFLplusplus/afl-fuzz -x "$allowlist" -i "$script_dir/seeds/" -o out -m none -d -- "$script_dir/build/simple_crash" >> "$result_afl"
/AFLplusplus/afl-fuzz  -i /exercises/exercise1/seeds/ -o out -m none -d -- /exercises/exercise1/build/simple_crash


echo "done"