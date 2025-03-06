// Copyright 2025, William MacDonald, All Rights Reserved.

/// TODO: might be able to simplify quite a bit by using the seed only for random world gen, and not send the whole vector, then may only ever need to send POD, nothing complicated, option 2 in claude chat

#pragma once

#include <flatbuffers/flatbuffers.h>
#include <vector>
#include <type_traits>
#include <cstdint>

template <typename T>
class NetworkSerializer
{
    // Type trait to detect if T is a vector
    template <typename U>
    static constexpr bool is_vector_v =
        std::is_same_v<U, std::vector<typename U::value_type>>;

public:
    // // Serialize a single item
    // static std::vector<uint8_t> serialize(const T& item) {
    //     flatbuffers::FlatBufferBuilder builder;

    //     // Use memcpy for POD (Plain Old Data) types
    //     if constexpr (std::is_trivially_copyable_v<T>) {
    //         auto dataOffset = builder.CreateVector(
    //             reinterpret_cast<const uint8_t*>(&item),
    //             sizeof(T)
    //         );

    //         // Create a wrapper to store the raw data
    //         auto rootOffset = CreateRawDataFB(builder, dataOffset);
    //         builder.Finish(rootOffset);
    //     }
    //     // Handle vector serialization
    //     else if constexpr (is_vector_v<T>) {
    //         using ElementType = typename T::value_type;

    //         std::vector<flatbuffers::Offset<void>> elementOffsets;
    //         for (const auto& element : item) {
    //             // Recursively serialize each element
    //             auto serializedElement = serialize(element);
    //             auto elementOffset = builder.CreateVector(
    //                 serializedElement.data(),
    //                 serializedElement.size()
    //             );
    //             elementOffsets.push_back(elementOffset);
    //         }

    //         auto vectorOffset = builder.CreateVector(elementOffsets);
    //         auto rootOffset = CreateVectorDataFB(builder, vectorOffset);
    //         builder.Finish(rootOffset);
    //     }

    //     // Convert to vector
    //     return std::vector<uint8_t>(
    //         builder.GetBufferPointer(),
    //         builder.GetBufferPointer() + builder.GetSize()
    //     );
    // }

    // // Deserialize a single item
    // static T deserialize(const uint8_t* data, size_t size) {
    //     // Implement deserialization logic
    //     // This will be more complex and depend on the specific type
    //     if constexpr (std::is_trivially_copyable_v<T>) {
    //         T result;
    //         memcpy(&result, data, sizeof(T));
    //         return result;
    //     }
    //     else if constexpr (is_vector_v<T>) {
    //         // Recursive deserialization for vectors
    //         T result;
    //         // Implement vector deserialization
    //         return result;
    //     }
    // }
};