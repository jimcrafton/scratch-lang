#pragma once



namespace language {
	namespace typesystem {

		enum TypeDescriptor {
			typeUnknown = 0,
			typeRecord = 1,
			typeClass,
			typeBit1,
			typeInteger8 = 8,
			typeUInteger8 = 9,
			typeInteger16 = 16,
			typeUInteger16 = 17,
			typeShort = typeInteger16,
			typeUShort = typeUInteger16,
			typeInteger32 = 32,
			typeUInteger32 = 33,
			typeInteger64 = 64,
			typeUInteger64 = 65,
			typeInteger128 = 128,
			typeUInteger128 = 129,

			typeDouble32 = 34,
			typeFloat = typeDouble32,
			typeDouble64 = 66,
			typeDouble = typeDouble64,
			typeBool = 100,
			typeString,
			typeArray,
			typeDictionary,
			typeMessage,
		};

		



		typedef  bool boolT;


		typedef  int8_t int8T;
		typedef  uint8_t uint8T;
		typedef  int16_t int16T;
		typedef  uint16_t uint16T;
		typedef  int32_t int32T;
		typedef  uint32_t uint32T;
		typedef  int64_t int64T;
		typedef  uint64_t uint64T;

		struct Type128 {
			int64T lo;
			int64T hi;
		};

		struct TypeU128 {
			uint64T lo;
			uint64T hi;
		};


		typedef  Type128 int128T;
		typedef  TypeU128 uint128T;


		typedef  float float32T;
		typedef double double64T;
	}
}