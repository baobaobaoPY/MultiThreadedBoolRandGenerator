#include <fmt/core.h>
#include <paused.h>
#include <iostream>
#include <random>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <regex>


const unsigned short thread_count{32};  // CPU线程数-逻辑处理器数量
const unsigned long long batch_size{5000};  // 每个线程每次处理的n次任务量
const unsigned long long Imodel{thread_count * batch_size};

// 用xoshiro256**作为mt19937_64的替代，因为它比mt19937_64更快
class XorShift64Star {
private:
    uint64_t state;
public:
    using result_type = uint64_t;
    
    XorShift64Star(uint64_t seed = 1) : state(seed) {
        if (state == 0) state = 1;  // 避免全零情况
    }
    
    static constexpr uint64_t min() { return 0; }
    static constexpr uint64_t max() { return UINT64_MAX; }
    
    uint64_t operator()() {
        state ^= state >> 12;
        state ^= state << 25;
        state ^= state >> 27;
        return state * 0x2545F4914F6CDD1DULL;
    }
};

// 线程执行函数-移除引用传递，直接返回结果
struct ThreadResult {
    unsigned long long true_count;
    unsigned long long false_count;
};

ThreadResult random_task(XorShift64Star& engine, unsigned long long batches) {
    ThreadResult result{0, 0};

    // 预计算总迭代次数，避免更多循环产生的开销
    const unsigned long long total_iterations{batches * batch_size};

    for (unsigned long long i{0}; i < total_iterations; ++i) {
        // 使用位运算判断奇偶性，比bernoulli_distribution更快
        if (engine() & 1) {
            ++result.true_count;
        } else {
            ++result.false_count;
        }
    }

    return result;
};

void input_handler(unsigned long long& total_iterations) { 
    while (true) {
        fmt::print("请输入{}的倍数：", Imodel);
        std::string input;
        std::cin >> input;

        if (std::regex_match(input, std::regex("(^\\d+$)"))) {
            total_iterations = std::stoull(input);

            if (total_iterations % Imodel == 0) {
                if (total_iterations == 0) {
                    AWML::paused("exit(0)");
                } else {
                    break;
                }
            } else {
                fmt::print("\x1b[91m输入有误，输入值必须是{}的倍数！\x1b[0m\n", Imodel);
                AWML::paused();
                fmt::print("\033c");
            }
        } else {
            fmt::print("\x1b[91m输入有误，输入必须为整数类型！\x1b[0m\n");
            AWML::paused();
            fmt::print("\033c");
        }
    }
};

int main() {
#ifdef _WIN32
    system("chcp 65001");  // 使用65001代码页，使Windows平台终端支持转义字符
#endif
    fmt::print("\033c");

    // 使用input_handler函数处理用户交互
    unsigned long long total_iterations{0};
    input_handler(total_iterations);

    // 分配任务
    const unsigned long long iterations_per_thread{total_iterations / thread_count};
    const unsigned long long batches_per_thread{iterations_per_thread / batch_size};

    fmt::print("\n");

    // 原子计数器-只保留最终结果
    std::atomic<unsigned long long> global_true{0};
    std::atomic<unsigned long long> global_false{0};

    // 预生成种子，避免在线程中创建
    std::random_device rd;
    std::vector<uint64_t> seeds(thread_count);
    for (size_t i{0}; i < thread_count; ++i) {
        seeds[i] = (static_cast<uint64_t>(rd()) << 32) | rd() | (i + 1);
    }

    std::vector<std::thread> threads;
    threads.reserve(thread_count);  // 预分配内存

    // 创建开始计时器
    auto start_time{std::chrono::high_resolution_clock::now()};

    for (unsigned short i{0}; i < thread_count; ++i) {
        threads.emplace_back([&, i, seed{seeds[i]}] {
            // 每个线程独立享用高速伪随机数生成器
            XorShift64Star engine(seed);

            // 执行随机任务
            ThreadResult result{random_task(engine, batches_per_thread)};

            // 在最后进行一次原子操作，以减少竞争
            global_true.fetch_add(result.true_count, std::memory_order_relaxed);
            global_false.fetch_add(result.false_count, std::memory_order_relaxed);
        });
    }

    // 等待所有线程任务完成
    for (auto& t : threads) {
        t.join();
    }

    // 添加计时器终点并核算耗时
    auto end_time{std::chrono::high_resolution_clock::now()};
    std::chrono::duration<double> duration{end_time - start_time};
    double seconds{duration.count()};

    // 输出最终生成结果
    fmt::print("\n\n生成结果为：true`\x1b[92m{}\x1b[0m`, false`\x1b[91m{}\x1b[0m`\n", 
               global_true.load(), global_false.load());

    // 输出整个过程所花费的时间
    fmt::print("耗时：\x1b[94m{:.6f}\x1b[0ms\n", seconds);

    AWML::paused("exit(0)");
    return 0;
}
