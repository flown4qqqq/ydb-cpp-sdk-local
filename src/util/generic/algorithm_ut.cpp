#include <src/library/testing/unittest/registar.h>

#include <ydb-cpp-sdk/util/generic/algorithm.h>
#include "hash.h"
#include "hash_multi_map.h"
#include "string.h"

static auto isOne = [](char c) { return c == '1'; };

Y_UNIT_TEST_SUITE(TAlgorithm) {
    Y_UNIT_TEST(AnyTest) {
        UNIT_ASSERT(0 == AnyOf(std::string_view("00"), isOne));
        UNIT_ASSERT(1 == AnyOf(std::string_view("01"), isOne));
        UNIT_ASSERT(1 == AnyOf(std::string_view("10"), isOne));
        UNIT_ASSERT(1 == AnyOf(std::string_view("11"), isOne));
        UNIT_ASSERT(0 == AnyOf(std::string_view(), isOne));

        const char array00[]{'0', '0'};
        UNIT_ASSERT(0 == AnyOf(array00, isOne));
        const char array01[]{'0', '1'};
        UNIT_ASSERT(1 == AnyOf(array01, isOne));
    }

    Y_UNIT_TEST(AllOfTest) {
        UNIT_ASSERT(0 == AllOf(std::string_view("00"), isOne));
        UNIT_ASSERT(0 == AllOf(std::string_view("01"), isOne));
        UNIT_ASSERT(0 == AllOf(std::string_view("10"), isOne));
        UNIT_ASSERT(1 == AllOf(std::string_view("11"), isOne));
        UNIT_ASSERT(1 == AllOf(std::string_view(), isOne));

        const char array01[]{'0', '1'};
        UNIT_ASSERT(0 == AllOf(array01, isOne));
        const char array11[]{'1', '1'};
        UNIT_ASSERT(1 == AllOf(array11, isOne));
    }

    Y_UNIT_TEST(CountIfTest) {
        UNIT_ASSERT(3 == CountIf(std::string_view("____1________1____1_______"), isOne));
        UNIT_ASSERT(5 == CountIf(std::string_view("1____1________1____1_______1"), isOne));
        UNIT_ASSERT(0 == CountIf(std::string_view("___________"), isOne));
        UNIT_ASSERT(0 == CountIf(std::string_view(), isOne));
        UNIT_ASSERT(1 == CountIf(std::string_view("1"), isOne));

        const char array[] = "____1________1____1_______";
        UNIT_ASSERT(3 == CountIf(array, isOne));
    }

    Y_UNIT_TEST(CountTest) {
        UNIT_ASSERT(3 == Count("____1________1____1_______", '1'));
        UNIT_ASSERT(3 == Count(std::string_view("____1________1____1_______"), '1'));
        UNIT_ASSERT(5 == Count(std::string_view("1____1________1____1_______1"), '1'));
        UNIT_ASSERT(0 == Count(std::string_view("___________"), '1'));
        UNIT_ASSERT(0 == Count(std::string_view(), '1'));
        UNIT_ASSERT(1 == Count(std::string_view("1"), '1'));

        const char array[] = "____1________1____1_______";
        UNIT_ASSERT(3 == Count(array, '1'));
    }

    struct TStrokaNoCopy: std::string {
    public:
        TStrokaNoCopy(const char* p)
            : std::string(p)
        {
        }

    private:
        TStrokaNoCopy(const TStrokaNoCopy&);
        void operator=(const TStrokaNoCopy&);
    };

    Y_UNIT_TEST(CountOfTest) {
        UNIT_ASSERT_VALUES_EQUAL(CountOf(1, 2), 0);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(1, 1), 1);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(2, 4, 5), 0);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(2, 4, 2), 1);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(3, 3, 3), 2);

        // Checking comparison of different types.
        UNIT_ASSERT_VALUES_EQUAL(CountOf(0x61, 'x', 'y', 'z'), 0);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(0x61, 'a', 'b', 'c', 0x61), 2);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(0x61, 'a', 'b', 'c', 0x61ll), 2);

        // std::string and const char *
        UNIT_ASSERT_VALUES_EQUAL(CountOf(std::string("xyz"), "123", "poi"), 0);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(std::string("xyz"), "123", "poi", "xyz"), 1);

        // std::string and std::string_view
        UNIT_ASSERT_VALUES_EQUAL(CountOf(std::string("xyz"), std::string_view("123"), std::string_view("poi")), 0);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(std::string("xyz"), std::string_view("123"), std::string_view("poi"),
                                         std::string_view("xyz")),
                                 1);

        // std::string_view and const char *
        UNIT_ASSERT_VALUES_EQUAL(CountOf(std::string_view("xyz"), "123", "poi"), 0);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(std::string_view("xyz"), "123", "poi", "xyz"), 1);

        // std::string_view and std::string
        UNIT_ASSERT_VALUES_EQUAL(CountOf(std::string_view("xyz"), std::string("123"), std::string("poi")), 0);
        UNIT_ASSERT_VALUES_EQUAL(CountOf(std::string_view("xyz"), std::string("123"), std::string("poi"),
                                         std::string("xyz")),
                                 1);
    }

    Y_UNIT_TEST(EqualToOneOfTest) {
        UNIT_ASSERT(1 == EqualToOneOf(1, 1, 2));
        UNIT_ASSERT(1 == EqualToOneOf(2, 1, 2));
        UNIT_ASSERT(0 == EqualToOneOf(3, 1, 2));
        UNIT_ASSERT(1 == EqualToOneOf(1, 1));
        UNIT_ASSERT(0 == EqualToOneOf(1, 2));
        UNIT_ASSERT(0 == EqualToOneOf(3));

        //test, that EqualToOneOf can compare different types, and don't copy objects:
        TStrokaNoCopy x("x");
        TStrokaNoCopy y("y");
        TStrokaNoCopy z("z");
        const char* px = "x";
        const char* py = "y";
        const char* pz = "z";

        UNIT_ASSERT(1 == EqualToOneOf(x, px, py));
        UNIT_ASSERT(1 == EqualToOneOf(y, px, py));
        UNIT_ASSERT(1 == EqualToOneOf(y, px, y));
        UNIT_ASSERT(1 == EqualToOneOf(y, x, py));
        UNIT_ASSERT(0 == EqualToOneOf(z, px, py));
        UNIT_ASSERT(1 == EqualToOneOf(px, x, y));
        UNIT_ASSERT(1 == EqualToOneOf(py, x, y));
        UNIT_ASSERT(0 == EqualToOneOf(pz, x, y));
    }

    template <class TTestConstPtr>
    void TestFindPtrFoundValue(int j, TTestConstPtr root) {
        if (j == 3) {
            UNIT_ASSERT(root && *root == 3);
        } else if (j == 4) {
            UNIT_ASSERT(root == nullptr);
        } else {
            ythrow yexception() << "invalid param " << j;
        }
    }

    template <class TTestConstPtr>
    void TestFindIfPtrFoundValue(int j, TTestConstPtr root) {
        if (j == 3) {
            UNIT_ASSERT(root == nullptr);
        } else if (j == 4) {
            UNIT_ASSERT(root && *root == 2);
        } else {
            ythrow yexception() << "invalid param " << j;
        }
    }

    struct TVectorNoCopy: std::vector<int> {
    public:
        TVectorNoCopy() = default;

    private:
        TVectorNoCopy(const TVectorNoCopy&);
        void operator=(const TVectorNoCopy&);
    };

    Y_UNIT_TEST(FindPtrTest) {
        TVectorNoCopy v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        int array[3] = {1, 2, 3};
        const int array_const[3] = {1, 2, 3};

        //test (const, non-const) * (iterator, vector, array) * (found, not found) variants.
        // value '3' is in container, value '4' is not
        for (int j = 3; j <= 4; ++j) {
            TestFindPtrFoundValue<int*>(j, FindPtr(v, j));
            TestFindPtrFoundValue<int*>(j, FindPtr(v.begin(), v.end(), j));
            const TVectorNoCopy& q = v;
            TestFindPtrFoundValue<const int*>(j, FindPtr(q, j));
            TestFindPtrFoundValue<const int*>(j, FindPtr(q.begin(), q.end(), j));
            TestFindPtrFoundValue<int*>(j, FindPtr(array, j));
            TestFindPtrFoundValue<const int*>(j, FindPtr(array_const, j));
        }
    }

    Y_UNIT_TEST(FindIfPtrTest) {
        TVectorNoCopy v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        int array[3] = {1, 2, 3};
        const int array_const[3] = {1, 2, 3};

        //test (const, non-const) * (iterator, vector, array) * (found, not found) variants.
        // search, that 2*2 == 4, but there is no value 'x' in array that (x*x == 3)
        for (int j = 3; j <= 4; ++j) {
            TestFindIfPtrFoundValue<int*>(j, FindIfPtr(v, [j](int i) { return i * i == j; }));
            TestFindIfPtrFoundValue<int*>(j, FindIfPtr(v.begin(), v.end(), [j](int i) { return i * i == j; }));
            const TVectorNoCopy& q = v;
            TestFindIfPtrFoundValue<const int*>(j, FindIfPtr(q, [j](int i) { return i * i == j; }));

            TestFindIfPtrFoundValue<const int*>(j, FindIfPtr(q.begin(), q.end(), [j](int i) { return i * i == j; }));
            TestFindIfPtrFoundValue<int*>(j, FindIfPtr(array, [j](int i) { return i * i == j; }));
            TestFindIfPtrFoundValue<const int*>(j, FindIfPtr(array_const, [j](int i) { return i * i == j; }));
        }
    }

    Y_UNIT_TEST(FindIndexTest) {
        TVectorNoCopy v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        UNIT_ASSERT_EQUAL(0, FindIndex(v, 1));
        UNIT_ASSERT_EQUAL(1, FindIndex(v, 2));
        UNIT_ASSERT_EQUAL(2, FindIndex(v, 3));
        UNIT_ASSERT_EQUAL(NPOS, FindIndex(v, 42));

        int array[3] = {1, 2, 3};

        UNIT_ASSERT_EQUAL(0, FindIndex(array, 1));
        UNIT_ASSERT_EQUAL(1, FindIndex(array, 2));
        UNIT_ASSERT_EQUAL(2, FindIndex(array, 3));
        UNIT_ASSERT_EQUAL(NPOS, FindIndex(array, 42));

        std::vector<int> empty;
        UNIT_ASSERT_EQUAL(NPOS, FindIndex(empty, 0));
    }

    Y_UNIT_TEST(FindIndexIfTest) {
        TVectorNoCopy v;
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);

        UNIT_ASSERT_EQUAL(0, FindIndexIf(v, [](int x) { return x == 1; }));
        UNIT_ASSERT_EQUAL(1, FindIndexIf(v, [](int x) { return x == 2; }));
        UNIT_ASSERT_EQUAL(2, FindIndexIf(v, [](int x) { return x == 3; }));
        UNIT_ASSERT_EQUAL(NPOS, FindIndexIf(v, [](int x) { return x == 42; }));

        int array[3] = {1, 2, 3};

        UNIT_ASSERT_EQUAL(0, FindIndexIf(array, [](int x) { return x == 1; }));
        UNIT_ASSERT_EQUAL(1, FindIndexIf(array, [](int x) { return x == 2; }));
        UNIT_ASSERT_EQUAL(2, FindIndexIf(array, [](int x) { return x == 3; }));
        UNIT_ASSERT_EQUAL(NPOS, FindIndexIf(array, [](int x) { return x == 42; }));

        std::vector<int> empty;
        UNIT_ASSERT_EQUAL(NPOS, FindIndexIf(empty, [](int x) { return x == 3; }));
    }

    Y_UNIT_TEST(SortUniqueTest) {
        {
            std::vector<std::string> v;
            SortUnique(v);
            UNIT_ASSERT_EQUAL(v, std::vector<std::string>());
        }

        {
            const char* ar[] = {"345", "3", "123", "2", "23", "3", "2"};
            std::vector<std::string> v(ar, ar + Y_ARRAY_SIZE(ar));
            SortUnique(v);

            const char* suAr[] = {"123", "2", "23", "3", "345"};
            std::vector<std::string> suV(suAr, suAr + Y_ARRAY_SIZE(suAr));

            UNIT_ASSERT_EQUAL(v, suV);
        }
    }

    Y_UNIT_TEST(EraseTest) {
        std::vector<int> data = {5, 4, 3, 2, 1, 0};
        std::vector<int> expected = {5, 4, 2, 1, 0};
        Erase(data, 3);
        UNIT_ASSERT_EQUAL(data, expected);
    }

    Y_UNIT_TEST(EraseIfTest) {
        std::vector<int> data = {5, 4, 3, 2, 1, 0};
        std::vector<int> expected = {2, 1, 0};
        EraseIf(data, [](int i) { return i >= 3; });
        UNIT_ASSERT_EQUAL(data, expected);
    }

    Y_UNIT_TEST(EraseNodesIfTest) {
        std::map<int, int> map{{1, 1}, {2, 2}, {3, 5}};
        std::map<int, int> expectedMap{{1, 1}};
        EraseNodesIf(map, [](auto p) { return p.first >= 2; });
        UNIT_ASSERT_EQUAL(map, expectedMap);

        std::multimap<int, int> multiMap{{1, 1}, {1, 3}, {2, 2}, {3, 5}};
        std::multimap<int, int> expectedMultiMap{{1, 1}, {1, 3}};
        EraseNodesIf(multiMap, [](auto p) { return p.first >= 2; });
        UNIT_ASSERT_EQUAL(multiMap, expectedMultiMap);

        TSet<int> set{1, 2, 3, 4, 5, 6, 7};
        TSet<int> expectedSet{1, 3, 5, 7};
        EraseNodesIf(set, [](int i) { return i % 2 == 0; });
        UNIT_ASSERT_EQUAL(set, expectedSet);

        TMultiSet<int> multiSet{1, 1, 2, 3, 4, 4, 4, 5, 5, 5, 6, 7};
        TMultiSet<int> expectedMultiSet{1, 1, 3, 5, 5, 5, 7};
        EraseNodesIf(multiSet, [](int i) { return i % 2 == 0; });
        UNIT_ASSERT_EQUAL(multiSet, expectedMultiSet);

        THashMap<int, int> hashMap{{1, 0}, {3, 0}, {4, 0}, {10, 0}, {2, 0}, {5, 2}};
        THashMap<int, int> expectedHashMap{{1, 0}, {3, 0}, {5, 2}};
        EraseNodesIf(hashMap, [](auto p) { return p.first % 2 == 0; });
        UNIT_ASSERT_EQUAL(hashMap, expectedHashMap);

        THashMultiMap<int, int> hashMultiMap{{1, 0}, {3, 0}, {4, 0}, {10, 0}, {2, 0}, {5, 0}, {1, 0}, {1, 0}, {2, 0}, {2, 2}};
        THashMultiMap<int, int> expectedHashMultiMap{{1, 0}, {1, 0}, {1, 0}, {3, 0}, {5, 0}};
        EraseNodesIf(hashMultiMap, [](auto p) { return p.first % 2 == 0; });
        UNIT_ASSERT_EQUAL(hashMultiMap, expectedHashMultiMap);
    }

    Y_UNIT_TEST(NthElementTest) {
        {
            std::vector<std::string> v;
            NthElement(v.begin(), v.begin(), v.end());
            UNIT_ASSERT_EQUAL(v, std::vector<std::string>());
        }

        {
            int data[] = {3, 2, 1, 4, 6, 5, 7, 9, 8};
            std::vector<int> testVector(data, data + Y_ARRAY_SIZE(data));

            size_t medianInd = testVector.size() / 2;

            NthElement(testVector.begin(), testVector.begin() + medianInd, testVector.end());
            UNIT_ASSERT_EQUAL(testVector[medianInd], 5);

            NthElement(testVector.begin(), testVector.begin() + medianInd, testVector.end(), [](int lhs, int rhs) { return lhs > rhs; });
            UNIT_ASSERT_EQUAL(testVector[medianInd], 5);
        }

        {
            const char* data[] = {"3", "234", "1231", "333", "545345", "11", "111", "55", "66"};
            std::vector<std::string> testVector(data, data + Y_ARRAY_SIZE(data));

            size_t medianInd = testVector.size() / 2;
            NthElement(testVector.begin(), testVector.begin() + medianInd, testVector.end());

            auto median = testVector.begin() + medianInd;
            for (auto it0 = testVector.begin(); it0 != median; ++it0) {
                for (auto it1 = median; it1 != testVector.end(); ++it1) {
                    UNIT_ASSERT(*it0 <= *it1);
                }
            }
        }
    }

    Y_UNIT_TEST(BinarySearchTest) {
        {
            std::vector<std::string> v;
            bool test = BinarySearch(v.begin(), v.end(), "test");
            UNIT_ASSERT_EQUAL(test, false);
        }

        {
            int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

            bool test = BinarySearch(data, data + Y_ARRAY_SIZE(data), 2);
            UNIT_ASSERT_EQUAL(test, true);

            test = BinarySearch(data, data + Y_ARRAY_SIZE(data), 10);
            UNIT_ASSERT_EQUAL(test, false);
        }

        {
            std::vector<size_t> data = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};

            bool test = BinarySearch(data.begin(), data.end(), (size_t)9, TGreater<size_t>());
            UNIT_ASSERT_EQUAL(test, true);

            test = BinarySearch(data.begin(), data.end(), (size_t)11, TGreater<size_t>());
            UNIT_ASSERT_EQUAL(test, false);

            test = BinarySearch(data.rbegin(), data.rend(), (size_t)1);
            UNIT_ASSERT_EQUAL(test, true);
        }
    }

    Y_UNIT_TEST(EqualRangeTest) {
        {
            std::vector<std::string> v;
            using PairOfVector = std::pair<std::vector<std::string>::iterator, std::vector<std::string>::iterator>;
            PairOfVector tmp = EqualRange(v.begin(), v.end(), "tmp");

            UNIT_ASSERT_EQUAL(tmp.first, tmp.second);
            UNIT_ASSERT_EQUAL(tmp.first, v.end());
        }

        {
            int data[] = {1, 1, 1, 1, 1, 2, 2, 3, 3, 3, 3, 4, 5};
            using PairOfInt = std::pair<int*, int*>;
            PairOfInt tmp = EqualRange(data, data + Y_ARRAY_SIZE(data), 3);

            UNIT_ASSERT_EQUAL(tmp.second - tmp.first, 4);
            UNIT_ASSERT_EQUAL(tmp.first - data, 7);
            UNIT_ASSERT_EQUAL(data + Y_ARRAY_SIZE(data) - tmp.second, 2);
        }

        {
            std::vector<size_t> data = {9, 9, 8, 8, 8, 5, 4, 3, 3, 0, 0};

            using PairOfVector = std::pair<std::vector<size_t>::iterator, std::vector<size_t>::iterator>;
            PairOfVector tmp = EqualRange(data.begin(), data.end(), 8, TGreater<size_t>());

            UNIT_ASSERT_EQUAL(tmp.first - data.begin(), 2);
            UNIT_ASSERT_EQUAL(tmp.second - tmp.first, 3);

            using PairOfVectorReverse = std::pair<std::vector<size_t>::reverse_iterator, std::vector<size_t>::reverse_iterator>;
            PairOfVectorReverse tmpR = EqualRange(data.rbegin(), data.rend(), (size_t)0);

            UNIT_ASSERT_EQUAL(tmpR.first, data.rbegin());
            UNIT_ASSERT_EQUAL(tmpR.second - tmpR.first, 2);
        }
    }

    Y_UNIT_TEST(AdjacentFindTest) {
        std::vector<int> v0;
        UNIT_ASSERT_EQUAL(AdjacentFind(v0), v0.end());

        std::vector<int> v1 = {1};
        UNIT_ASSERT_EQUAL(AdjacentFind(v1), v1.end());

        const int v2[] = {8, 7, 6, 6, 5, 5, 5, 4, 3, 2, 1};
        UNIT_ASSERT_EQUAL(AdjacentFind(v2), std::begin(v2) + 2);

        std::vector<std::string_view> v3 = {"six", "five", "four", "three", "two", "one"};
        UNIT_ASSERT_EQUAL(AdjacentFind(v3), v3.end());

        std::vector<int> v4 = {1, 1, 1, 1, 1};
        for (;;) {
            if (auto it = AdjacentFind(v4); it == v4.end()) {
                break;
            } else {
                *it += 1;
            }
        }
        UNIT_ASSERT_VALUES_EQUAL(v4, (std::vector<int>{5, 4, 3, 2, 1}));
    }

    Y_UNIT_TEST(AdjacentFindByTest) {
        std::vector<int> v0;
        UNIT_ASSERT_EQUAL(AdjacentFindBy(v0, std::negate<int>()), v0.end());

        std::vector<int> v1 = {1};
        UNIT_ASSERT_EQUAL(AdjacentFindBy(v1, std::negate<int>()), v1.end());

        const int v2[] = {8, 7, 6, 6, 5, 5, 5, 4, 3, 2, 1};
        UNIT_ASSERT_EQUAL(AdjacentFindBy(v2, std::negate<int>()), std::begin(v2) + 2);
        UNIT_ASSERT_EQUAL(AdjacentFindBy(v2, [](const auto& e) { return e / 8; }), std::begin(v2) + 1);

        std::vector<std::string_view> v3 = {"six", "five", "four", "three", "two", "one"};
        UNIT_ASSERT_EQUAL(AdjacentFind(v3), v3.end());
        UNIT_ASSERT_EQUAL(AdjacentFindBy(v3, std::mem_fn(&std::string_view::size)), v3.begin() + 1);

        std::vector<int> v4 = {101, 201, 301, 401, 501};
        for (;;) {
            if (auto it = AdjacentFindBy(v4, [](int a) { return a % 10; }); it == v4.end()) {
                break;
            } else {
                *it += 1;
            }
        }
        UNIT_ASSERT_VALUES_EQUAL(v4, (std::vector<int>{105, 204, 303, 402, 501}));
    }

    Y_UNIT_TEST(IsSortedTest) {
        std::vector<int> v0;
        UNIT_ASSERT_VALUES_EQUAL(IsSorted(v0.begin(), v0.end()), true);

        std::vector<int> v1 = {1, 2, 3, 4, 5, 5, 5, 6, 6, 7, 8};
        UNIT_ASSERT_VALUES_EQUAL(IsSorted(v1.begin(), v1.end()), true);
        UNIT_ASSERT_VALUES_EQUAL(IsSorted(v1.begin(), v1.end(), TLess<int>()), true);
        UNIT_ASSERT_VALUES_EQUAL(IsSorted(v1.begin(), v1.end(), TGreater<int>()), false);

        std::vector<int> v2 = {1, 2, 1};
        UNIT_ASSERT_VALUES_EQUAL(IsSorted(v2.begin(), v2.end()), false);
        UNIT_ASSERT_VALUES_EQUAL(IsSorted(v2.begin(), v2.end(), TLess<int>()), false);
        UNIT_ASSERT_VALUES_EQUAL(IsSorted(v2.begin(), v2.end(), TGreater<int>()), false);
    }

    Y_UNIT_TEST(IsSortedByTest) {
        std::vector<int> v0;
        UNIT_ASSERT_VALUES_EQUAL(IsSortedBy(v0.begin(), v0.end(), std::negate<int>()), true);
        UNIT_ASSERT_VALUES_EQUAL(IsSortedBy(v0, std::negate<int>()), true);

        std::vector<int> v1 = {1};
        UNIT_ASSERT_VALUES_EQUAL(IsSortedBy(v1.begin(), v1.end(), std::negate<int>()), true);
        UNIT_ASSERT_VALUES_EQUAL(IsSortedBy(v1, std::negate<int>()), true);

        std::vector<int> v2 = {8, 7, 6, 6, 5, 5, 5, 4, 3, 2, 1};
        UNIT_ASSERT_VALUES_EQUAL(IsSortedBy(v2.begin(), v2.end(), std::negate<int>()), true);
        UNIT_ASSERT_VALUES_EQUAL(IsSortedBy(v2, std::negate<int>()), true);

        std::vector<int> v3 = {1, 2, 1};
        UNIT_ASSERT_VALUES_EQUAL(IsSortedBy(v3.begin(), v3.end(), std::negate<int>()), false);
        UNIT_ASSERT_VALUES_EQUAL(IsSortedBy(v3, std::negate<int>()), false);
    }

    Y_UNIT_TEST(SortTestTwoIterators) {
        std::vector<int> collection = {10, 2, 7};
        Sort(collection.begin(), collection.end());
        std::vector<int> expected = {2, 7, 10};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(SortTestTwoIteratorsAndComparator) {
        std::vector<int> collection = {10, 2, 7};
        Sort(collection.begin(), collection.end(), [](int l, int r) { return l > r; });
        std::vector<int> expected = {10, 7, 2};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(SortTestContainer) {
        std::vector<int> collection = {10, 2, 7};
        Sort(collection);
        std::vector<int> expected = {2, 7, 10};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(SortTestContainerAndComparator) {
        std::vector<int> collection = {10, 2, 7};
        Sort(collection, [](int l, int r) { return l > r; });
        std::vector<int> expected = {10, 7, 2};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(StableSortTestTwoIterators) {
        std::vector<int> collection = {10, 2, 7};
        StableSort(collection.begin(), collection.end());
        std::vector<int> expected = {2, 7, 10};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(StableSortTestTwoIteratorsAndComparator) {
        std::vector<int> collection = {404, 101, 106, 203, 102, 205, 401};
        StableSort(collection.begin(), collection.end(), [](int l, int r) { return (l / 100) < (r / 100); });
        std::vector<int> expected = {101, 106, 102, 203, 205, 404, 401};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(StableSortTestContainer) {
        std::vector<int> collection = {10, 2, 7};
        StableSort(collection);
        std::vector<int> expected = {2, 7, 10};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(StableSortTestContainerAndComparator) {
        std::vector<int> collection = {404, 101, 106, 203, 102, 205, 401};
        StableSort(collection, [](int l, int r) { return (l / 100) < (r / 100); });
        std::vector<int> expected = {101, 106, 102, 203, 205, 404, 401};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(SortByTest) {
        std::vector<int> collection = {10, 2, 7};
        SortBy(collection, [](int x) { return -x; });
        std::vector<int> expected = {10, 7, 2};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(StableSortByTest) {
        std::vector<int> collection = {404, 101, 106, 203, 102, 205, 401};
        StableSortBy(collection, [](int x) { return x / 100; });
        std::vector<int> expected = {101, 106, 102, 203, 205, 404, 401};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(SortUniqueByTest) {
        std::vector<int> collection = {404, 101, 101, 203, 101, 203, 404};
        StableSortUniqueBy(collection, [](int x) { return x / 100; });
        std::vector<int> expected = {101, 203, 404};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(StableSortUniqueByTest) {
        std::vector<int> collection = {404, 101, 106, 203, 102, 205, 401};
        StableSortUniqueBy(collection, [](int x) { return x / 100; });
        std::vector<int> expected = {101, 203, 404};
        UNIT_ASSERT_VALUES_EQUAL(collection, expected);
    }

    Y_UNIT_TEST(IotaTest) {
        std::vector<int> v(10);

        Iota(v.begin(), v.end(), 0);
        UNIT_ASSERT_VALUES_EQUAL(v[0], 0);
        UNIT_ASSERT_VALUES_EQUAL(v[5], 5);
        UNIT_ASSERT_VALUES_EQUAL(v[9], 9);

        Iota(v.begin() + 2, v.begin() + 5, 162);
        UNIT_ASSERT_VALUES_EQUAL(v[0], 0);
        UNIT_ASSERT_VALUES_EQUAL(v[3], 163);
        UNIT_ASSERT_VALUES_EQUAL(v[9], 9);
    }

    Y_UNIT_TEST(CopyNTest) {
        int data[] = {1, 2, 3, 4, 8, 7, 6, 5};
        const size_t vSize = 10;
        std::vector<int> result(10, 0);
        size_t toCopy = 5;

        std::vector<int>::iterator iter = CopyN(data, toCopy, result.begin());
        UNIT_ASSERT_VALUES_EQUAL(iter - result.begin(), toCopy);
        UNIT_ASSERT_VALUES_EQUAL(result.size(), 10);
        for (size_t idx = 0; idx < toCopy; ++idx) {
            UNIT_ASSERT_VALUES_EQUAL(data[idx], result[idx]);
        }
        for (size_t idx = toCopy; idx < vSize; ++idx) {
            UNIT_ASSERT_VALUES_EQUAL(result[idx], 0);
        }

        toCopy = 8;
        const size_t start = 1;
        result.assign(vSize, 0);
        iter = CopyN(data, toCopy, result.begin() + start);
        UNIT_ASSERT_VALUES_EQUAL(iter - result.begin(), start + toCopy);
        for (size_t idx = 0; idx < start; ++idx) {
            UNIT_ASSERT_VALUES_EQUAL(result[idx], 0);
        }
        for (size_t idx = 0; idx < toCopy; ++idx) {
            UNIT_ASSERT_VALUES_EQUAL(result[start + idx], data[idx]);
        }
        for (size_t idx = start + toCopy; idx < vSize; ++idx) {
            UNIT_ASSERT_VALUES_EQUAL(result[idx], 0);
        }
    }

    Y_UNIT_TEST(CopyIfTest) {
        const size_t count = 9;
        int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        const size_t vSize = 10;
        std::vector<int> v(vSize, 0);

        std::vector<int>::iterator iter = CopyIf(data, data + count, v.begin(), [](int x) { return !(x % 3); });
        UNIT_ASSERT_VALUES_EQUAL(v.size(), vSize);
        UNIT_ASSERT_VALUES_EQUAL(iter - v.begin(), 3);
        v.resize(iter - v.begin());
        for (size_t idx = 0; idx < v.size(); ++idx) {
            UNIT_ASSERT_VALUES_EQUAL(v[idx], 3 * (idx + 1));
        }
    }

    Y_UNIT_TEST(MinMaxElementTest) {
        std::vector<int> v(10);
        Iota(v.begin(), v.end(), 0);
        UNIT_ASSERT_EQUAL(*MinMaxElement(v.begin(), v.end()).first, 0);
        UNIT_ASSERT_EQUAL(*MinMaxElement(v.begin(), v.end()).second, 9);

        v[3] = -2;
        v[7] = 11;
        UNIT_ASSERT_EQUAL(*MinMaxElement(v.begin(), v.end()).first, -2);
        UNIT_ASSERT_EQUAL(*MinMaxElement(v.begin(), v.end()).second, 11);
    }

    Y_UNIT_TEST(MinMaxTest) {
        std::pair<int, int> p1 = MinMax(5, 12);
        UNIT_ASSERT_EQUAL(p1.first, 5);
        UNIT_ASSERT_EQUAL(p1.second, 12);

        std::pair<std::string, std::string> p2 = MinMax(std::string("test"), std::string("data"));
        UNIT_ASSERT_EQUAL(p2.first, std::string("data"));
        UNIT_ASSERT_EQUAL(p2.second, std::string("test"));
    }

    Y_UNIT_TEST(TestMaxElementBy) {
        const int array[] = {1, 2, 5, 3, 4, 5};
        UNIT_ASSERT_VALUES_EQUAL(*MaxElementBy(array, [](int x) {
            return x * x;
        }), 5);

        const std::vector<int> vec(array, array + Y_ARRAY_SIZE(array));
        UNIT_ASSERT_VALUES_EQUAL(*MaxElementBy(vec, [](int x) {
            return -1.0 * x;
        }), 1);

        int arrayMutable[] = {1, 2, 5, 3, 4, 5};
        auto maxPtr = MaxElementBy(arrayMutable, [](int x) { return x; });
        *maxPtr += 100;
        UNIT_ASSERT_VALUES_EQUAL(*maxPtr, 105);

        auto identity = [](char x) {
            return x;
        };
        auto singleElementSequence = {'z'};
        UNIT_ASSERT_VALUES_EQUAL(*MaxElementBy(singleElementSequence, identity), 'z');

        const std::string strings[] = {"one", "two", "three", "four"};
        UNIT_ASSERT_STRINGS_EQUAL(*MaxElementBy(strings, [](std::string s) { return s.size(); }), "three");
    }

    Y_UNIT_TEST(TestMinElementBy) {
        const int array[] = {2, 3, 4, 1, 5};
        UNIT_ASSERT_VALUES_EQUAL(*MinElementBy(array, [](int x) -> char {
            return 'a' + x;
        }), 1);

        const std::vector<int> vec(std::begin(array), std::end(array));
        UNIT_ASSERT_VALUES_EQUAL(*MinElementBy(vec, [](int x) {
            return -x;
        }), 5);

        int arrayMutable[] = {1, 2, 5, 3, 4, 5};
        auto minPtr = MinElementBy(arrayMutable, [](int x) { return x; });
        *minPtr += 100;
        UNIT_ASSERT_VALUES_EQUAL(*minPtr, 101);

        auto identity = [](char x) {
            return x;
        };
        auto singleElementSequence = {'z'};
        UNIT_ASSERT_VALUES_EQUAL(*MinElementBy(singleElementSequence, identity), 'z');

        const std::vector<std::string_view> strings = {"one", "two", "three", "four"};
        auto stringLength = [](std::string_view s) {
            return s.size();
        };
        UNIT_ASSERT_STRINGS_EQUAL(*MinElementBy(strings, stringLength), "one");
        UNIT_ASSERT_STRINGS_EQUAL(*MinElementBy(strings.rbegin(), strings.rend(), stringLength), "two");
    }

    Y_UNIT_TEST(MaxElementByReturnsEndForEmptyRange) {
        const std::vector<int> empty;
        UNIT_ASSERT_EQUAL(MaxElementBy(empty, [](int) { return 0; }), empty.end());
    }

    Y_UNIT_TEST(MaxElementByDoesntCallFunctorForEmptyRange) {
        const std::vector<int> empty;
        auto functor = [](int) {
            UNIT_ASSERT(false);
            return 0;
        };
        MaxElementBy(empty, functor);
    }

    Y_UNIT_TEST(MinElementByReturnsEndForEmptyRange) {
        const std::vector<int> empty;
        UNIT_ASSERT_EQUAL(MinElementBy(empty, [](int) { return 0; }), empty.end());
    }

    Y_UNIT_TEST(MinElementByDoesntCallFunctorForEmptyRange) {
        const std::vector<int> empty;
        auto functor = [](int) {
            UNIT_ASSERT(false);
            return 0;
        };
        MinElementBy(empty, functor);
    }

    Y_UNIT_TEST(TestApplyToMany) {
        int res = 0;
        ApplyToMany([&res](auto v) { res += v; }, 1, 2, 3, 4, 5);
        UNIT_ASSERT_EQUAL(res, 15);

        struct TVisitor {
            TVisitor(int& acc)
                : Acc(acc)
            {
            }
            void operator()(const std::string& s) {
                Acc += s.size();
            }
            void operator()(int v) {
                Acc += v * 2;
            }
            int& Acc;
        };
        std::string s{"8-800-555-35-35"};
        ApplyToMany(TVisitor{res = 0}, 1, s, 5, s);
        UNIT_ASSERT_EQUAL(res, 12 + 2 * static_cast<int>(s.size()));
    }

    Y_UNIT_TEST(TestTupleForEach) {
        ForEach(std::tuple<>{}, [&](auto) { UNIT_ASSERT(false); });
        auto t = std::make_tuple(5, 6, 2, 3, 6);
        ForEach(t, [](auto& v) { v *= -1; });
        UNIT_ASSERT_EQUAL(t, std::make_tuple(-5, -6, -2, -3, -6));
    }

    Y_UNIT_TEST(TestTupleAllOf) {
        UNIT_ASSERT(AllOf(std::tuple<>{}, [](auto) { return false; }));
        UNIT_ASSERT(!AllOf(std::make_tuple(1, 2, 0, 4, 5), [&](auto v) { UNIT_ASSERT_LT(v, 3); return 0 != v; }));
        UNIT_ASSERT(AllOf(std::make_tuple(1, 2, 3, 4, 5), [](auto v) { return 0 != v; }));
        {
            auto pred = std::function<bool(int)>([x = std::vector<int>(1, 0)](auto v) { return x.front() != v; });
            UNIT_ASSERT(AllOf(std::make_tuple(1, 2), pred));
            UNIT_ASSERT(AllOf(std::make_tuple(1, 2), pred));
        }
        {
            auto ts = std::make_tuple(std::string{"foo"}, std::string{"bar"});
            auto pred = [](auto s) { return s.size() == 3; };
            UNIT_ASSERT_VALUES_EQUAL(AllOf(ts, pred), AllOf(ts, pred));
        }
    }

    Y_UNIT_TEST(TestTupleAnyOf) {
        UNIT_ASSERT(!AnyOf(std::tuple<>{}, [](auto) { return true; }));
        UNIT_ASSERT(AnyOf(std::make_tuple(0, 1, 2, 3, 4), [&](auto v) { UNIT_ASSERT_LT(v, 2); return 1 == v; }));
        UNIT_ASSERT(AnyOf(std::make_tuple(1, 2, 3, 4, 5), [](auto v) { return 5 == v; }));
        auto pred = std::function<bool(int)>([x = std::vector<int>(1, 0)](auto v) { return x.front() == v; });
        UNIT_ASSERT(!AnyOf(std::make_tuple(1, 2), pred));
        UNIT_ASSERT(!AnyOf(std::make_tuple(1, 2), pred));
        {
            auto ts = std::make_tuple(std::string{"f"}, std::string{"bar"});
            auto pred = [](auto s) { return s.size() == 3; };
            UNIT_ASSERT_VALUES_EQUAL(AnyOf(ts, pred), AnyOf(ts, pred));
        }
    }

    Y_UNIT_TEST(FindIfForContainer) {
        using std::begin;
        using std::end;

        int array[] = {1, 2, 3, 4, 5};
        UNIT_ASSERT_EQUAL(FindIf(array, [](int x) { return x == 1; }), begin(array));
        UNIT_ASSERT_EQUAL(FindIf(array, [](int x) { return x > 5; }), end(array));

        std::vector<int> vector = {1, 2, 3, 4, 5};
        UNIT_ASSERT_EQUAL(FindIf(vector, [](int x) { return x == 1; }), begin(vector));
        UNIT_ASSERT_EQUAL(FindIf(vector, [](int x) { return x > 5; }), end(vector));

        // Compilability test. Check if the returned iterator is non const
        auto iter = FindIf(vector, [](int x) { return x == 1; });
        *iter = 5;

        // Compilability test. Check if the returned iterator is const. Should not compile
        const std::vector<int> constVector = {1, 2, 3, 4, 5};
        auto constIter = FindIf(constVector, [](int x) { return x == 1; });
        Y_UNUSED(constIter);
        // *constIter = 5;
    }

    struct TRange {
    };

    const TRange* begin(const TRange& r) {
        return &r;
    }

    const TRange* end(const TRange& r) {
        return &r + 1;
    }

    Y_UNIT_TEST(FindIfForUserType) {
        // Compileability test. Should work for user types with begin/end overloads
        TRange range;
        auto i = FindIf(range, [](auto) { return false; });
        Y_UNUSED(i);
    }

    Y_UNIT_TEST(TestLowerBoundBy) {
        using TIntPairs = std::vector<std::pair<i32, i32>>;

        auto data = TIntPairs{{1, 5}, {3, 2}, {3, 4}, {8, 0}, {5, 4}};
        auto getKey = [](const auto& x) { return x.second; };

        StableSortBy(data, getKey);

        auto it = LowerBoundBy(data.begin(), data.end(), 4, getKey);
        UNIT_ASSERT(it != data.end());
        UNIT_ASSERT_EQUAL(it->second, 4);
        UNIT_ASSERT_EQUAL(it->first, 3);

        UNIT_ASSERT(it > data.begin());
        UNIT_ASSERT_EQUAL((it - 1)->second, 2);

        UNIT_ASSERT((it + 1) < data.end());
        UNIT_ASSERT_EQUAL((it + 1)->second, 4);
    }

    Y_UNIT_TEST(TestUpperBoundBy) {
        using TIntPairs = std::vector<std::pair<i32, i32>>;

        auto data = TIntPairs{{1, 5}, {3, 2}, {3, 4}, {8, 0}, {5, 4}};
        auto getKey = [](const auto& x) { return x.second; };

        StableSortBy(data, getKey);

        auto it = UpperBoundBy(data.begin(), data.end(), 4, getKey);
        UNIT_ASSERT(it != data.end());
        UNIT_ASSERT_EQUAL(it->second, 5);
        UNIT_ASSERT_EQUAL(it->first, 1);

        UNIT_ASSERT(it > data.begin());
        UNIT_ASSERT_EQUAL((it - 1)->second, 4);

        UNIT_ASSERT((it + 1) == data.end());
    }

    Y_UNIT_TEST(TestFindInContainer) {
        std::vector<int> v = {1, 2, 1000, 15, 100};
        UNIT_ASSERT(Find(v, 5) == v.end());
        UNIT_ASSERT(Find(v, 1) == v.begin());
        UNIT_ASSERT(Find(v, 100) == v.end() - 1);
    }

    Y_UNIT_TEST(AccumulateWithBinOp) {
        std::vector<int> v = {1, 2, 777};
        UNIT_ASSERT_VALUES_EQUAL(std::string("begin;1;2;777"), Accumulate(v, std::string("begin"), [](auto&& a, auto& b) { return a + ";" + ToString(b); }));
    }
}
