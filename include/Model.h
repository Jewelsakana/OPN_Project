#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <stdexcept>
#include <memory>

// Model基类：提供基础错误处理接口和资源管理
class Model {
public:
    virtual ~Model() = default;

    // 基础错误处理接口
    virtual void validate() const {
        // 子类可以实现验证逻辑
    }

    // 获取模型名称（用于调试和日志）
    virtual std::string getName() const {
        return "Model";
    }

    // 重置模型状态
    virtual void reset() {
        // 子类可以实现状态重置逻辑
    }

    // 检查模型是否处于有效状态
    virtual bool isValid() const {
        return true;
    }

    // 异常包装器：执行操作并捕获异常，提供统一的错误处理
    template<typename Func, typename... Args>
    auto safeExecute(Func func, Args&&... args) {
        try {
            validate();
            return func(std::forward<Args>(args)...);
        } catch (const std::exception& e) {
            handleException(e);
            throw; // 重新抛出异常，让调用者处理
        }
    }

protected:
    // 异常处理函数（子类可以重写）
    virtual void handleException(const std::exception& e) const {
        // 基础实现：什么也不做，子类可以添加日志记录等
    }
};

#endif // MODEL_H