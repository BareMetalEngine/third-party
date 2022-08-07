/***
* Inferno Engine v4
* Written by Tomasz Jonarski (RexDex)
* Source code licensed under LGPL 3.0 license
***/

#include "build.h"

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/RuntimeDyld.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include <llvm/ExecutionEngine/ExecutionEngine.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "llvm/Support/raw_ostream.h"

//--

using namespace llvm;
using namespace llvm::orc;

namespace prv
{

	class Compiler
	{
	public:
		Compiler(LLVMContext& llvm)
			: m_llvm(llvm)
		{
			InitializeNativeTarget();
			InitializeNativeTargetAsmPrinter();
			InitializeNativeTargetAsmParser();

			m_module = std::make_unique<Module>("module", m_llvm);
			m_module->setDataLayout(EngineBuilder().selectTarget()->createDataLayout());
		}

   		Function* buildAddMulFunction()
		{
			auto typeInt32 = IntegerType::get(m_llvm, 32);

			auto funcCallee = m_module->getOrInsertFunction("mul_add", llvm::AttributeList(), typeInt32, typeInt32, typeInt32, typeInt32);

			Function* func = cast<Function>(funcCallee.getCallee());
			func->setLinkage(llvm::GlobalValue::ExternalLinkage);
			func->setCallingConv(CallingConv::C);
			
			Value* args[3];

			{
				Function::arg_iterator it = func->arg_begin();
				args[0] = it++;
				args[0]->setName("x");
				args[1] = it++;
				args[1]->setName("y");
				args[2] = it++;
				args[2]->setName("z");
			}

			{  
				IRBuilder<> builder(m_llvm);

				BasicBlock* block = BasicBlock::Create(m_llvm, "entry", func);
				builder.SetInsertPoint(block);

				Value* tmp = builder.CreateBinOp(Instruction::Mul, args[0], args[1], "tmp");
				Value* tmp2 = builder.CreateBinOp(Instruction::Add, tmp, args[2], "tmp2");
				builder.CreateRet(tmp2);
			}

			std::string str;
			raw_string_ostream ss(str);
			EXPECT_FALSE(llvm::verifyFunction(*func, &ss)) << str;

			
			return func;
		}

		std::unique_ptr<Module> extractModule()
		{
			return std::move(m_module);
		}

	private:
		LLVMContext& m_llvm;
		std::unique_ptr<Module> m_module;
	};
	
	class Printer
	{
	public:
		Printer(LLVMContext& llvm)
			: m_llvm(llvm)
			, m_ostream(m_text)
		{
			m_module = std::make_unique<Module>("my cool jit", llvm);

			m_fpm = std::make_unique<legacy::FunctionPassManager>(m_module.get());
			m_fpm->add(createPrintFunctionPass(m_ostream));
			m_fpm->doInitialization();
		}

		std::string print(Function* f)
		{
			m_text.clear();
			m_fpm->run(*f);
			fprintf(stderr, "%s\n", m_text.c_str());
			return std::string(m_text.c_str());
		}

	public:
		LLVMContext& m_llvm;
		std::unique_ptr<Module> m_module;
		std::unique_ptr<legacy::FunctionPassManager> m_fpm;
		std::string m_text;
		raw_string_ostream m_ostream;
	};

	class Optimizer
	{
	public:
		Optimizer(LLVMContext& llvm)
			: m_llvm(llvm)
		{
			m_module = std::make_unique<Module>("my cool jit", llvm);

			m_fpm = std::make_unique<legacy::FunctionPassManager>(m_module.get());
			m_fpm->add(createInstructionCombiningPass()); // Do simple "peephole" optimizations and bit-twiddling optzns.
			m_fpm->add(createReassociatePass()); // Reassociate expressions.
			m_fpm->add(createGVNPass()); // Eliminate Common SubExpressions.
			m_fpm->add(createCFGSimplificationPass()); // Simplify the control flow graph (deleting unreachable blocks, etc).
			m_fpm->doInitialization();
		}

		void optimize(Function* f)
		{
			m_fpm->run(*f);
		}

	private:
		LLVMContext& m_llvm;
		std::unique_ptr<Module> m_module;
		std::unique_ptr<legacy::FunctionPassManager> m_fpm;
	};

} // prv

TEST(LLVM, BasicIntegration)
{
	LLVMContext l;
	prv::Compiler ctx(l);
}

TEST(LLVM, BuildSimpleFunction)
{
	LLVMContext l;
	prv::Compiler ctx(l);
	ctx.buildAddMulFunction();
}

static const char* PrintSimpleFunctionExpectedTxt = R""""(
define i32 @mul_add(i32 %x, i32 %y, i32 %z) {
entry:
  %tmp = mul i32 %x, %y
  %tmp2 = add i32 %tmp, %z
  ret i32 %tmp2
}
)"""";

TEST(LLVM, PrintSimpleFunction)
{
	LLVMContext l;
	prv::Compiler ctx(l);
	auto* f= ctx.buildAddMulFunction();

	prv::Printer printer(l);
	auto str = printer.print(f);
	EXPECT_STREQ(PrintSimpleFunctionExpectedTxt, str.c_str());
}

TEST(LLVM, CreateOptimizer)
{
	LLVMContext l;
	prv::Optimizer optimizer(l);
}

TEST(LLVM, OptimizeSimpleFunction)
{
	LLVMContext l;

	prv::Compiler ctx(l);
	auto* f = ctx.buildAddMulFunction();

	prv::Optimizer optimizer(l);
	optimizer.optimize(f);
}

TEST(LLVM, CreateSimpleJIT)
{
	auto JIT = LLJITBuilder().create();
	ASSERT_TRUE((bool)JIT);
}

TEST(LLVM, RunSimpleFunction)
{
	LLVMContext l;

	prv::Compiler ctx(l);
	auto* f = ctx.buildAddMulFunction();

	{
		prv::Optimizer optimizer(l);
		optimizer.optimize(f);
	}

	{
		auto JIT = LLJITBuilder().create();
		ASSERT_TRUE((bool)JIT);

		auto localCtx = std::make_unique<LLVMContext>();
		JIT->get()->addIRModule(ThreadSafeModule(std::move(ctx.extractModule()), std::move(localCtx)));

		// Look up the JIT'd code entry point.
		auto symbol = JIT->get()->lookup("mul_add");
		ASSERT_TRUE((bool)symbol);

		// Cast the entry point address to a function pointer.
		auto* symbolPtr = (int(*)(int, int, int))symbol->getValue();
		ASSERT_TRUE(symbolPtr != nullptr);

		// Call into JIT'd code.
		auto ret = symbolPtr(2, 3, 5); // (2*3) + 5 -> 11
		EXPECT_EQ(11, ret);
	}
}

//--
