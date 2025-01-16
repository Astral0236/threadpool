#include<iostream>
#include<thread>
#include<mutex>
#include<queue>
#include<condition_variable>
#include<vector>
#include<functional>
class threadpool
{
public:
    threadpool(int numthreads):stop(false){
        for(int i=0;i<numthreads;i++){
            threads.emplace_back([this]{
                while(1){
                    std::unique_lock<std::mutex> lock(mtx);
                    condition.wait(lock,[this] {
                        return !tasks.empty() || stop ;
                    });

                    if(stop&&tasks.empty()) { return;}

                    std::function<void()> task(std::move(tasks.front()));
                    tasks.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    }
    ~threadpool(){
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop=true;
        }
        condition.notify_all();
        for(auto& t:threads){
            t.join(); 
        }
    }
    template<class F,class... Args> //函数模板里面你取两个取址符是万能引用
    void addtask(F&& f, Args&&... args){
        std::function<void()>task = 
                    std::bind(std::forward<F>(f),std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(mtx);
      -     tasks.emplace(std::move(task));
        }

        condition.notify_one();  
    }
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;

    std::mutex mtx;
    std::condition_variable condition; 

    bool stop;

};
int main()
{
    //std::cout<<"hello world\n";
    threadpool pool(4);
    for(int i=0;i<10;i++) {
        pool.addtask([i] {
            std::cout<<"task:"<<i<<" is running\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout<<"task:"<<i<<" is done\n";
        });
    }
    return 0;
}