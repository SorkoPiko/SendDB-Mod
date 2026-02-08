#pragma once

template <typename T> requires requires { std::hash<T>(); }
 void hash_combine(size_t& seed, const T& value) {
    seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}