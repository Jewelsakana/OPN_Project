#ifndef OBSERVE_H
#define OBSERVE_H

// 前向声明Event，避免循环依赖
class Event;

// Observe接口：用于实现观察者模式，定义Update(Event e)，以便日志模块监听
class Observe {
public:
    virtual ~Observe() = default;

    // 更新方法，当被观察对象状态变化时调用
    virtual void update(const Event& e) = 0;
};

#endif // OBSERVE_H