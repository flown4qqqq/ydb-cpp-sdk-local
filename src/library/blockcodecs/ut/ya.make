UNITTEST_FOR(src/library/blockcodecs)

FORK_TESTS()

FORK_SUBTESTS()

SPLIT_FACTOR(40)

TIMEOUT(300)

SIZE(MEDIUM)

SRCS(
    codecs_ut.cpp
)

END()