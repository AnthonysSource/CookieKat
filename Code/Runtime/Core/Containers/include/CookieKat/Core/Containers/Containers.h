#pragma once

#include <vector>
#include <array>
#include <optional>
#include <queue>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace CKE {
	template <typename T>
	using Func = std::function<T>;

	template <typename T>
	using Vector = std::vector<T>;

	template <typename T, size_t Size>
	using Array = std::array<T, Size>;

	template <typename T>
	using Stack = std::stack<T>;

	template <typename T>
	using Queue = std::queue<T>;

	template <typename T>
	using Set = std::unordered_set<T>;

	template <typename K, typename V>
	using Map = std::unordered_map<K, V>;

	template <typename T, typename K>
	using Pair = std::pair<T, K>;

	template <typename T>
	using Optional = std::optional<T>;
}

namespace CKE {
	class MapUtils
	{
	public:
		template <typename K, typename V>
		inline static Vector<K> VectorFromMapKeys(Map<K, V> const& map) {
			Vector<K> ids{};
			ids.reserve(map.size());
			for (auto&& pair : map) {
				ids.push_back(pair.first);
			}
			return ids;
		}
	};
}
