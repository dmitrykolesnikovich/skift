#pragma once

#include <karm-meta/cvrp.h>

#include "_prelude.h"

#define try$(EXPR)                \
    ({                            \
        auto __expr = (EXPR);     \
        if (!__expr) {            \
            return __expr.none(); \
        }                         \
        __expr.take();            \
    })

namespace Karm {

template <typename T>
concept Tryable = requires(T t) {
    {!t};
    {t.none()};
    {t.unwrap()};
};

auto tryOr(Tryable auto opt, Meta::RemoveRef<decltype(opt.unwrap())> defaultValue) -> Meta::RemoveRef<decltype(opt.unwrap())> {
    if (!opt) {
        return defaultValue;
    }

    return opt.unwrap();
}

auto tryOrElse(Tryable auto opt, auto defaultValue) -> decltype(opt.unwrap()) {
    if (!opt) {
        return defaultValue();
    }

    return opt.unwrap();
}

} // namespace Karm
