

#include "Runtime.h"



namespace runtime {
		


	/*
	Runtime Runtime::rtInstance;
	Runtime::Runtime()
	{
		
	}

	Runtime::~Runtime()
	{

	}

	bool Runtime::init()
	{
		

		return true;
	}

	bool Runtime::finish()
	{
		
		return true;
	}
	


	int Runtime::mainCrtEntryPoint(int argc, char** argv)
	{
		std::vector<std::string> args;

		for (int i = 0; i < argc;i++) {

			args.push_back( std::string(argv[i]) );
		}

		return Runtime::mainRtEntryPoint(args);
	}


	int Runtime::mainRtEntryPoint(const std::vector<std::string>& args)
	{ 
		MainReturnValues result = mainOK;

		try {
			if (Runtime::rtInstance.init()) {
				if (nullptr != Runtime::rtInstance.main) {
					int res = (*Runtime::rtInstance.main)(&Runtime::rtInstance, args);
					result = (MainReturnValues)res;
				}
				else {
					throw Runtime::Error(MainReturnValues::mainNoMainEntryPointSpecified, "Runtime main function is null");
				}

				if (!Runtime::rtInstance.finish()) {
					result = MainReturnValues::mainRuntimeFinishFailed;
				}
			}
			else {
				result = MainReturnValues::mainRuntimeInitFailed;
			}				
		}
		catch (const Runtime::Error& e) {
			result = e.errorCode;
		}
		catch (...) {
			result = MainReturnValues::mainGeneralException;
		}
		
		return (int)result;
	}


	const datatypes::objectinfo& Runtime::getInfo(const datatypes::object& obj)
	{
		if (obj.infoptr.infoIdx < Runtime::rtInstance.objInfoList.size()) {
			return Runtime::rtInstance.objInfoList[obj.infoptr.infoIdx];
		}
			

		return datatypes::objectinfo::nil();
	}


	const datatypes::Record* Runtime::getRecord(const datatypes::object& obj)
	{
		const datatypes::Record* result = nullptr;

		if (obj.infoptr.infoIdx < Runtime::rtInstance.recordInfoList.size()) {
			result = Runtime::rtInstance.recordInfoList[obj.infoptr.infoIdx];
		}
			

		return result; 
	}

	const datatypes::Class* Runtime::getClass(const datatypes::object& obj)
	{
		const datatypes::Class* result = nullptr;

		if (obj.infoptr.infoIdx < Runtime::rtInstance.classInfoList.size()) {
			result = Runtime::rtInstance.classInfoList[obj.infoptr.infoIdx];
		}


		return result;
	}




		


	void Runtime::registerClass(const datatypes::string& name)
	{

	}

	void Runtime::registerRecord(const datatypes::string& name)
	{
			
	}

	void Runtime::registerType(typesystem::TypeDescriptor type)
	{
		datatypes::objectinfo info;
		info.type = type;
		switch (info.type) {
			case typesystem::TypeDescriptor::typeBool :{
				info.size = 1;
			} break;

			case typesystem::TypeDescriptor::typeInteger8: {
				info.size = 1;
			} break;

			case typesystem::TypeDescriptor::typeUInteger8: {
				info.size = 1;
			} break;

			case typesystem::TypeDescriptor::typeInteger16: {
				info.size = 2;
			} break;

			case typesystem::TypeDescriptor::typeUInteger16: {
				info.size = 2;
			} break;

			case typesystem::TypeDescriptor::typeInteger32: {
				info.size = 4;
			} break;

			case typesystem::TypeDescriptor::typeUInteger32: {
				info.size = 4;
			} break;

			case typesystem::TypeDescriptor::typeInteger64: {
				info.size = 8;
			} break;

			case typesystem::TypeDescriptor::typeUInteger64: {
				info.size = 8;
			} break;

			case typesystem::TypeDescriptor::typeInteger128: {
				info.size = 16;
			} break;

			case typesystem::TypeDescriptor::typeUInteger128: {
				info.size = 16;
			} break;

			case typesystem::TypeDescriptor::typeFloat:  {
				info.size = 4;
			} break;

			case typesystem::TypeDescriptor::typeDouble: {
				info.size = 8;
			} break;

			default: {
				info.size = 0;
			} break;
		}
		Runtime::rtInstance.objInfoList.push_back(info);

	}
	*/
}
