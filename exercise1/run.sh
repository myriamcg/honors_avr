# Install dependencies if necessary

# AFL_DIR=$(pwd)/src
# if [ -f "src/install_dependencies.sh" ]; then
#   echo "Installing dependencies"
#   cd src || exit
#   ./install_dependencies.sh
#   cd .. || exit
# fi

# cd build || exit
# if [ -f "../src/compile.sh" ]; then
#  CC="$AFL_DIR/afl-clang-fast" CXX="$AFL_DIR/afl-clang-fast++" ../src/compile.sh
# else
#  CC="$AFL_DIR/afl-clang-fast" CXX="$AFL_DIR/afl-clang-fast++" make ../src && make || exit
# fi

# echo "Running Cppcheck..."
# # cppcheck --enable=all exercise1/simple_crash.cpp 2> cppcheck_report.txt
# cppcheck simple_crash.cpp 2> cppcheck_report.txt
# echo "slay queen"

# status=$?
# nano cppcheck_report.txt

# denylist="denylist.txt"
#     grep -oP '^\[.*?\]:.*?(\b\w+\b)\(\)' cppcheck_report.txt | awk -F '(' '{print $2}' | awk '{print $1}' > $denylist
#     echo "oianglkank"
# if [ $status -ne 0 ]; then
#    echo "Cppcheck found issues. Check cppcheck_report.txt for details."
#
#    # Step 1.1: Extract problematic functions from Cppcheck report
#    echo "Generating denylist from Cppcheck report..."
  #  denylist="denylist.txt"
  #  grep -oP '^\[.*?\]:.*?(\b\w+\b)\(\)' cppcheck_report.txt | awk -F '(' '{print $2}' | awk '{print $1}' > $denylist
#    echo "oianglkank"
#
#    # Check if denylist is empty
#    #if [ ! -s $denylist ]; then
#    #    echo "No functions to denylist based on Cppcheck findings."
#    #    rm $denylist
#    #else
#    #    echo "Denylist created: $denylist"
#    #    cat $denylist
#    # fi
##else
##    echo "No issues found by Cppcheck."
##    denylist=""
##fi
# allowlist="$(pwd)/allowlist.txt"
# python3 problematic-files.py > "$allowlist"
# sort -u "$allowlist" -o "$allowlist"

control_flow_graph="$(pwd)control_flow_graph.txt"
python3 decision_tree.py > "$control_flow_graph"

decision_tree="$(pwd)decision_tree.txt"
python3 dec_tree_1.py > "$decision_tree"


cd ..
echo "Cppcheck done!"

# echo "Cppcheck found issues in the following functions"
# cat "$allowlist"

# echo "cfg  is"
# cat "$control_flow_graph"

echo "decision tree is"
cat "$decision_tree"



# add the file with problematic functions to AFL_LLVM_ALLOWLIST (environment variable)
echo "Compiling with AFL++ instrumentation..."
# if [ -n "$allowlist" ]; then
#     export AFL_LLVM_ALLOWLIST="$allowlist"
# fi



# /AFLplusplus/afl-fuzz -x "$allowlist" -i /exercises/exercise1/seeds/ -o out -m none -d -- /exercises/exercise1/build/simple_crash
echo "done"

