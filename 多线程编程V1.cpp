#include <fmt/core.h>  // 引入第三方库-高效输出
#include <paused.h>    // 引入自定义库-阻塞暂停
#include <iostream>
#include <random>
#include <vector>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <regex>


const unsigned short thread_count{32};       // CPU线程-逻辑处理器
const unsigned long long batch_size{5000};   // 每个线程每次处理n次任务
const unsigned long long Imodel{thread_count * batch_size};

// 线程执行
void random_task(std::mt19937_64& engine,
                 unsigned long long& local_true,
                 unsigned long long& local_false) {

    std::bernoulli_distribution dist(0.5);
    for (unsigned long long i{0}; i < batch_size; ++i) {
        if (dist(engine)) {
            ++local_true;
        } else {
            ++local_false;
        }
    }
}

// 处理用户输入
void input_handler(unsigned long long& total_iterations) { 
    while (true) {
        fmt::print("请输入{}的倍数：", Imodel);
        std::string input;
        std::cin >> input;

        /**
         * ^     表示字符串的开始
         * \\d+  表示一个或多个数字字符，即0-9
         * $     表示字符串的结束
        **/
        if (std::regex_match(input, std::regex("(^\\d+$)"))) {
            total_iterations = std::stoll(input);  // 转换字符串为整数

            if (total_iterations % Imodel == 0) {

                if (total_iterations == 0) {
                    AWML::paused("exit(0)");
                } else {
                    break;  // 正确输入的整数且同时满足%Imodel时，则跳出循环
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
}


int main (void) {
#ifdef _WIN32
    system("chcp 65001");
#endif
    fmt::print("\033c");
    unsigned long long total_iterations{0};
    std::atomic<unsigned long long> completed_count(0);

    // 调用`处理用户输入`的函数
    input_handler(total_iterations);

    // 分配任务给各线程
    const unsigned long long iterations_per_thread{total_iterations / thread_count};
    const unsigned long long batches_per_thread{iterations_per_thread / batch_size};

    fmt::print("\n");  // 换行保持控制台输出清爽

    // 计数器
    std::atomic<unsigned long long> global_true{0};
    std::atomic<unsigned long long> global_false{0};
    unsigned long long thread_local local_true{0};
    unsigned long long thread_local local_false{0};
    std::mutex counter_mutex;

    // 创建并开启计时器
    auto start_time{std::chrono::high_resolution_clock::now()};

    // 创建线程池
    std::vector<std::thread> threads;
    for (unsigned long long i{0}; i < thread_count; ++i) {
        threads.emplace_back([&, i] {
            // 每个线程享用独立的随机引擎
            #ifdef _WIN32
                std::random_device rd;
            #elif defined(__linux__) || defined(__APPLE__)
                std::random_device rd("/dev/urandom");
            #endif

            std::seed_seq seq{rd(), rd(), static_cast<uint32_t>(i)};
            thread_local std::mt19937_64 engine(seq);

            for (unsigned long long b{0}; b < batches_per_thread; ++b) {
                random_task(engine, local_true, local_false);

                // 非阻塞式进度更新
                unsigned long long current_completed \ 
                {completed_count.fetch_add(batch_size)};

                // 进度输出已经充分优化，即使注释也只能快上`0.3s至0.8s`
                // if ((current_completed + batch_size) % Imodel == 0) {
                //      std::lock_guard<std::mutex> lock(counter_mutex);
                //      fmt::print("已完成\x1b[94m{}\x1b[0m次真假随机生成！\r",
                //                 current_completed + batch_size);
                //  }
            }

            // 汇总本地计数
            std::lock_guard<std::mutex> lock(counter_mutex);
            global_true += local_true;
            global_false += local_false;
        });
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 添加计时器终点并核算耗时
    auto end_time{std::chrono::high_resolution_clock::now()};
    std::chrono::duration<double> duration{end_time - start_time};
    double seconds{duration.count()};

    fmt::print("\n\n生成结果为：true`\x1b[92m{}\x1b[0m`, false`\x1b[91m{}\x1b[0m`\n", 
               global_true.load(), global_false.load());
    fmt::print("耗时：\x1b[94m{:.6f}\x1b[0ms\n", seconds);


    AWML::paused("exit(0)");
    return 0;
}
