#pragma once

#include <functional>

/**
 * Helper to defer cleanup of resources that can be cancelled.
 */
class Defer {
public:
    Defer(std::function<void()> fun)
        : _fun(fun),
          _cancelled(false)
    {
    }

    ~Defer()
    {
        if (! _cancelled) {
            _fun();
        }
    }

    Defer(const Defer&) = delete;
    Defer &operator=(Defer&) = delete;

    void cancel() { _cancelled = true; }

private:
    std::function<void()> _fun;
    bool _cancelled;
};
