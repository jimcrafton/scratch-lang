#include "pch.h"

#include "runtime.h"



TEST(TestRuntimeObjects, Test_objectinfo_flags) {

	language::runtime::datatypes::objectinfo_flags flags;
	flags.setMemType(false);
	EXPECT_EQ(true, flags.isMemOnStack());
	EXPECT_EQ(false, flags.isMemOnHeap());

	flags.setMemType(true);
	EXPECT_EQ(false, flags.isMemOnStack());
	EXPECT_EQ(true, flags.isMemOnHeap());
	
}

TEST(TestRuntimeObjects, Test_objectinfo_sizes) {

	
	
	EXPECT_EQ(sizeof(uint32_t), sizeof(language::runtime::datatypes::objectinfo_flags));

	EXPECT_EQ(sizeof(uint64_t), sizeof(language::runtime::datatypes::objectinfo_ptr));
	
}
