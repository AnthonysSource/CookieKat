#pragma once

#include "ComponentArray.h"
#include "CookieKat/Core/Containers/Containers.h"
#include "IDs.h"

namespace CKE {
	struct IteratorsUtilities
	{
		template <size_t I = 0, typename... Ts>
		constexpr static inline void PopulateVectorWithComponentIDs(Vector<ComponentTypeID>& vec);

		template <size_t I = 0, typename... Ts>
		constexpr static inline void PopulateTupleWithComponents(std::tuple<Ts...>&       tuple,
		                                                         Vector<ComponentArray*>& componentArrays,
		                                                         u64                      row);
	};
}

namespace CKE {
	template <size_t I, typename... Ts>
	constexpr void IteratorsUtilities::PopulateVectorWithComponentIDs(Vector<ComponentTypeID>& vec) {
		if constexpr (I == sizeof...(Ts)) { return; }
		else {
			vec.emplace_back(ComponentStaticTypeID<std::tuple_element_t<I, std::tuple<Ts...>>>::s_CompID);
			PopulateVectorWithComponentIDs<I + 1, Ts...>(vec);
		}
	}

	template <size_t I, typename... Ts>
	constexpr void IteratorsUtilities::PopulateTupleWithComponents(std::tuple<Ts...>&       tuple,
	                                                               Vector<ComponentArray*>& componentArrays,
	                                                               u64                      row) {
		if constexpr (I == sizeof...(Ts)) { return; }
		else {
			std::get<I>(tuple) =
					(std::tuple_element_t<I, std::tuple<Ts...>>)
					componentArrays.at(I)->GetCompAtIndex(row);

			PopulateTupleWithComponents<I + 1>(tuple, componentArrays, row);
		}
	}
}
