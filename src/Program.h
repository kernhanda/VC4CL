/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#ifndef VC4CL_PROGRAM
#define VC4CL_PROGRAM

#include <vector>

#include "Object.h"
#include "Context.h"
#include "Bitfield.h"

namespace vc4cl
{
	enum class BuildStatus
	{
		//not yet built at all
		NOT_BUILD = 0,
		//compiled but not yet linked
		COMPILED = 1,
		//linked and compiled -> done
		DONE = 2
	};

	struct BuildInfo
	{
		cl_build_status status = CL_BUILD_NONE;
		std::string options;
		std::string log;
	};

	//THIS NEEDS TO HAVE THE SAME VALUES AS THE AddressSpace TYPE IN THE COMPILER!!
	enum class AddressSpace
	{
		GENERIC = 0,
		PRIVATE = 1,
		GLOBAL = 2,
		CONSTANT = 3,
		LOCAL = 4
	};

	/*
	 * NOTE: ParamInfo and KernelInfo need to map exactly to the corresponding types in the VC4C project!
	 */
	struct ParamInfo : private Bitfield<uint64_t>
	{
		ParamInfo(uint64_t val = 0) : Bitfield(val) { }

		//the size of this parameter in bytes (e.g. 4 for pointers)
		BITFIELD_ENTRY(Size, uint8_t, 0, Byte)
		//the number of components for vector-parameters
		BITFIELD_ENTRY(Elements, uint8_t, 8, Byte)
		BITFIELD_ENTRY(NameLength, uint16_t, 16, Short)
		BITFIELD_ENTRY(TypeNameLength, uint16_t, 32, Short)
		//whether this parameter is constant, only valid for pointers
		BITFIELD_ENTRY(Constant, bool, 48, Bit)
		//whether the memory behind this parameter is guaranteed to not be aligned (overlap with other memory areas), only valid for pointers
		BITFIELD_ENTRY(Restricted, bool, 49, Bit)
		//whether the memory behind this parameter is volatile, only valid for pointers
		BITFIELD_ENTRY(Volatile, bool, 50, Bit)
		//the parameter's address space, only valid for pointers
		//OpenCL default address space is "private"
		BITFIELD_ENTRY(AddressSpace, AddressSpace, 52, Quadruple)
		//whether this parameter is being read, only valid for pointers
		BITFIELD_ENTRY(Input, bool, 56, Bit)
		//whether this parameter is being written, only valid for pointers
		BITFIELD_ENTRY(Output, bool, 57, Bit)
		//whether this parameter is a pointer to data
		BITFIELD_ENTRY(Pointer, bool, 60, Bit)

		//the parameter name
		std::string name;
		//the parameter type-name, e.g. "<16 x i32>*"
		std::string type;
	};

	/*
	 * NOTE: ParamInfo and KernelInfo need to map exactly to the corresponding types in the VC4C project!
	 */
	struct KernelInfo : private Bitfield<uint64_t>
	{
		KernelInfo(uint64_t val = 0) : Bitfield(val) { }

		//the offset of the instruction belonging to the kernel, in instructions (8 byte)
		BITFIELD_ENTRY(Offset, uint16_t, 0, Short)
		//the number of 64-bit instructions in the kernel
		BITFIELD_ENTRY(Length, uint16_t, 16, Short)
		BITFIELD_ENTRY(NameLength, uint16_t, 32, Short)
		BITFIELD_ENTRY(ParamCount, uint16_t, 48, Short)

		//the kernel-name
		std::string name;
		//the work-group size specified at compile-time
		std::array<std::size_t,kernel_config::NUM_DIMENSIONS> compileGroupSizes;
		//the info for all explicit parameters
		std::vector<ParamInfo> params;

		size_t getExplicitUniformCount() const;
	};

	typedef void(CL_CALLBACK* BuildCallback)(cl_program program, void* user_data);

	class Program: public Object<_cl_program, CL_INVALID_PROGRAM>, public HasContext
	{
	public:
		Program(Context* context, const std::vector<char>& code, const cl_bool isBinary);
		~Program();


		CHECK_RETURN cl_int compile(const std::string& options, BuildCallback callback, void* userData);
		CHECK_RETURN cl_int link(const std::string& options, BuildCallback callback, void* userData);
		CHECK_RETURN cl_int getInfo(cl_program_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);
		CHECK_RETURN cl_int getBuildInfo(cl_program_build_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret);

		//the program's source, OpenCL C-code or LLVM IR / SPIR-V
		std::vector<char> sourceCode;
		//the machine-code, VC4C binary
		std::vector<char> binaryCode;
		//the global-data segment
		std::vector<char> globalData;

		BuildInfo buildInfo;

		//the kernel-infos, extracted from the VC4C binary
		//if this is set, the program is completely finished compiling
		std::vector<KernelInfo> kernelInfo;

		BuildStatus getBuildStatus() const;
	private:
		cl_int extractKernelInfo(cl_ulong** ptr, cl_uint* minKernelOffset);
	};

} /* namespace vc4cl */

#endif /* VC4CL_PROGRAM */
