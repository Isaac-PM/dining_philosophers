/*
Based on the explanation of the "Dining philosophers problem"
in this video by Troels Mortensen @ https://www.youtube.com/watch?v=w_Cug4_-7F0
*/

#include <array>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>

class Fork
{
public:
    Fork(uint id) : id(id), m_inUse(false)
    {
    }

    void pickUp()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this]()
                         { return !m_inUse; });
        m_inUse = true;
    }

    void putDown()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_inUse = false;
        m_condition.notify_one();
    }

    uint id;

private:
    bool m_inUse;
    std::mutex m_mutex;
    std::condition_variable m_condition;
};

class Philosopher
{
public:
    Philosopher(
        const std::string &name,
        Fork &fork1,
        Fork &fork2)
        : name(name),
          fork1(fork1),
          fork2(fork2)
    {
    }

    void run()
    {
        std::uniform_int_distribution<int> dist(0, 1);
        std::uniform_int_distribution<int> eatTime(1, 100);
        std::uniform_int_distribution<int> thinkTime(1, 10);
        while (true)
        {
            int action = dist(randEngine);

            if (action == 0)
            { // Eating
                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    std::cout << name << " wants to eat\n";
                }

                fork1.pickUp();
                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    std::cout << name << " picked up fork " << fork1.id << "\n";
                }

                fork2.pickUp();
                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    std::cout << name << " picked up fork " << fork2.id << "\n";
                    std::cout << name << " is now eating\n";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(eatTime(randEngine)));

                fork1.putDown();
                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    std::cout << name << " put down fork " << fork1.id << "\n";
                }

                fork2.putDown();
                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    std::cout << name << " put down fork " << fork2.id << "\n";
                }
            }
            else
            { // Thinking
                {
                    std::lock_guard<std::mutex> lock(outputMutex);
                    std::cout << name << " is thinking\n";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(thinkTime(randEngine)));
            }
        }
    }

    std::string name;
    Fork &fork1;
    Fork &fork2;
    std::mt19937 randEngine;
    static std::mutex outputMutex;
};

std::mutex Philosopher::outputMutex;

int main(void)
{
    Fork fork1(1);
    Fork fork2(2);
    Fork fork3(3);
    Fork fork4(4);
    Fork fork5(5);

    std::array<std::unique_ptr<Philosopher>, 5> philosophers;
    philosophers[0] = std::make_unique<Philosopher>("Philosopher 1", fork1, fork2);
    philosophers[1] = std::make_unique<Philosopher>("Philosopher 2", fork2, fork3);
    philosophers[2] = std::make_unique<Philosopher>("Philosopher 3", fork3, fork4);
    philosophers[3] = std::make_unique<Philosopher>("Philosopher 4", fork4, fork5);
    philosophers[4] = std::make_unique<Philosopher>("Philosopher 5", fork5, fork1);

    for (auto &philosopher : philosophers)
    {
        std::thread thread(&Philosopher::run, philosopher.get());
        thread.detach();
    }

    std::this_thread::sleep_for(std::chrono::seconds(50));

    return EXIT_SUCCESS;
}