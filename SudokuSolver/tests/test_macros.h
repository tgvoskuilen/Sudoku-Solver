#pragma once
#include <memory>
#include <vector>
#include <memory>
#include <iostream>
#include <chrono>

constexpr auto GREEN = "\033[92m";
constexpr auto RED = "\033[91m";
constexpr auto NC = "\033[0m";

#define EXPECT_ANY_THROW(cmd)                   \
   try {                                        \
     cmd;                                       \
     ++fails;                                   \
      std::cout << RED << "Expected '"          \
                << #cmd                         \
                << "' to throw, but it did not" \
                << NC << std::endl;             \
    } catch(std::exception& e) { } 

#define EXPECT_NO_THROW(cmd)                    \
   try { cmd; }                                 \
   catch(std::exception& e)                     \
   {                                            \
      ++fails;                                  \
      std::cout << RED << "Expected '"          \
                << #cmd                         \
                << "' not to throw, but it did" \
                << NC << std::endl;             \
   }

#define EXPECT_EQ(a,b)                          \
  if( a != b ) {                                \
     ++fails;                                   \
     std::cout << RED << "Expected " << #a      \
               << ", got " << #b                \
               << NC << std::endl;              \
  }

#define EXPECT_TRUE(a)                          \
  if( !a ) {                                    \
     ++fails;                                   \
     std::cout << RED                           \
         << "Expected true, but was false"      \
               << NC << std::endl;              \
  }

#define EXPECT_FALSE(a)                         \
  if( a ) {                                     \
     ++fails;                                   \
     std::cout << RED                           \
         << "Expected false, but was true"      \
               << NC << std::endl;              \
  }


class Test {
public:
    virtual void run() = 0;
    virtual std::string name() = 0;
    int fails = 0;
};

class TestRunner {
public:
    static TestRunner& getInstance() {
        static TestRunner t;
        return t;
    }

    TestRunner() = default;
    TestRunner(const TestRunner& rhs) = delete;
    TestRunner& operator=(const TestRunner& rhs) = delete;

    void add_test(std::shared_ptr<Test> t) {
        tests.push_back(t);
    }

    int run_all_tests() {
        int fails = 0;
        for (auto t : tests) {
            auto start = std::chrono::steady_clock::now();
            t->run();
            auto end = std::chrono::steady_clock::now();
            double elapsed_ms = 1e-3 * std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            fails += t->fails;
            if (t->fails > 0) {
                std::cout << RED << "[ fail ]: " << t->name() << " (" << elapsed_ms << " ms)" << NC << std::endl;
            }
            else {
                std::cout << GREEN << "[ pass ]: " << t->name() << " (" << elapsed_ms << " ms)" << NC << std::endl;
            }
        }

        if (fails > 0) {
            std::cout << RED << "FAILED " << fails << " TESTS" << NC << std::endl;
        }
        else {
            std::cout << GREEN << "ALL TESTS PASSED" << NC << std::endl;
        }

        return fails;
    }

private:
    std::vector<std::shared_ptr<Test>> tests;
};

template<class TestType>
void* MakeInfo() {
    auto& r = TestRunner::getInstance();
    r.add_test(std::make_shared<TestType>());
    return nullptr;
}

#define TEST(testname)                                  \
  class testname : public Test {                        \
    public:                                             \
      std::string name() override { return #testname; } \
      void run() override;                              \
    private:                                            \
      static void* const info;                          \
  };                                                    \
void* const testname::info = MakeInfo<testname>();      \
void testname::run()