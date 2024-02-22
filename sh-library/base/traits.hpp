#pragma once

template <typename T1, typename T2>
using add_result_type = decltype(T1{} + T2{});
