Command line used to find this crash:

/AFLplusplus/afl-fuzz -i /exercises/exercise1/seed2/ -o /exercises/exercise1/out -S fuzzer03 -m none -d -- /exercises/exercise1/build/simple_crash

If you can't reproduce a bug outside of afl-fuzz, be sure to set the same
memory limit. The limit used for this fuzzing session was 0 B.

Need a tool to minimize test cases before investigating the crashes or sending
them to a vendor? Check out the afl-tmin that comes with the fuzzer!

Found any cool bugs in open-source tools using afl-fuzz? If yes, please post
to https://github.com/AFLplusplus/AFLplusplus/issues/286 once the issues
 are fixed :)

