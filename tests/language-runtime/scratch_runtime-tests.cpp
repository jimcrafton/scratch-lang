#include "pch.h"

#include "scratch_runtime.h"




TEST(TestScratchRuntime, Test_runtime_number_sizes) {




	EXPECT_EQ(sizeof(boolT), 1);

	EXPECT_EQ(sizeof(int8T), 1);
	EXPECT_EQ(sizeof(uint8T), 1);

	EXPECT_EQ(sizeof(int16T), 2);
	EXPECT_EQ(sizeof(uint16T), 2);

	EXPECT_EQ(sizeof(int32T), 4);
	EXPECT_EQ(sizeof(uint32T), 4);

	EXPECT_EQ(sizeof(int64T), 8);
	EXPECT_EQ(sizeof(uint64T), 8);

	EXPECT_EQ(sizeof(Type128), 16);
	EXPECT_EQ(sizeof(TypeU128), 16);

	EXPECT_EQ(sizeof(float32T), 4);
	EXPECT_EQ(sizeof(double64T), 8);
}


TEST(TestScratchRuntime, Test_runtime_mem) {


	struct RuntimeMemory memInfo;
	memInfo.type = (RuntimeTypeDescriptor)980989;
	memInfo.memPtr = (void*)&printf;
	

	EXPECT_EQ(memInfo.type, (RuntimeTypeDescriptor)980989);
	EXPECT_EQ(memInfo.memPtr, (void*)&printf);

	

	Runtime_RuntimeMemory_init(&memInfo);

	EXPECT_EQ(memInfo.type, (RuntimeTypeDescriptor)0);
	EXPECT_EQ(memInfo.memPtr, (void*)0);
}


