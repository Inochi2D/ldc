//===-- gen/objcgen.cpp -----------------------------------------*- C++ -*-===//
//
//                         LDC – the LLVM D compiler
//
// This file is distributed under the BSD-style LDC license. See the LICENSE
// file for details.
//
//===----------------------------------------------------------------------===//
//
// Functions for generating Objective-C method calls.
//
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>
#include "llvm/ADT/StringMap.h"

struct ObjcSelector;
namespace llvm {
class Constant;
class GlobalVariable;
class Module;
class Triple;
}

// Forward decl.
class ClassDeclaration;
class FuncDeclaration;
class InterfaceDeclaration;
class VarDeclaration;
class Identifier;

typedef llvm::StringMap<llvm::GlobalVariable *> SymbolCache;
typedef std::vector<llvm::Constant *> ConstantList;

bool objc_isSupported(const llvm::Triple &triple);

// A mask needed to extract pointers from tagged Objective-C pointers.
// Only lower 35-bits in tagged pointers are used.
#define OBJC_PTRMASK 0x7ffffffff

// TODO: maybe make this slightly less cursed by actually
// poking the Objective-C library?
#define OBJC_METHOD_SIZEOF 24
#define OBJC_IVAR_ENTSIZE 32

// class is a metaclass
#define RO_META               (1<<0)

// class is a root class
#define RO_ROOT               (1<<1)

// Objective-C state tied to an LLVM module (object file).
class ObjCState {
public:
  ObjCState(llvm::Module &module) : module(module) { }

  // General
  llvm::GlobalVariable *getTypeEncoding(const Declaration& ty);

  // Classes
  llvm::GlobalVariable *getClassSymbol(const ClassDeclaration& cd, bool meta);
  llvm::GlobalVariable *getClassRoSymbol(const ClassDeclaration& cd, bool meta);
  llvm::GlobalVariable *getClassReference(const ClassDeclaration& cd);

  // Categories

  // Interface variables
  llvm::GlobalVariable *getIVarListFor(const ClassDeclaration& cd);
  llvm::GlobalVariable *getIVarSymbol(const ClassDeclaration& cd, const VarDeclaration& var);
  llvm::GlobalVariable *getIVarOffset(const ClassDeclaration& cd, const VarDeclaration& var, bool outputSymbol);

  // Methods
  llvm::GlobalVariable *getMethodVarType(const llvm::StringRef& ty);

  // Selector
  llvm::GlobalVariable *getSelector(const ObjcSelector &sel);

  // Protocols
  llvm::GlobalVariable *getProtocolListFor(const ClassDeclaration& cd);
  llvm::GlobalVariable *getProtocolSymbol(const InterfaceDeclaration& id);
  llvm::GlobalVariable *getProtocolReference(const InterfaceDeclaration& id);

  // Unmasks pointer to make sure it's not a tagged pointer.
  llvm::Value *unmaskPointer(llvm::Value *value);

  void finalize();

  // Section Names
  const char *imageInfoSection      = "__DATA,__objc_imageinfo, regular, no_dead_strip";

  // Lists
  const char *classListSection      = "__DATA,__objc_classlist, regular, no_dead_strip";
  const char *protoListSection      = "__DATA,__objc_protolist, regular, no_dead_strip";
  const char *catListSection        = "__DATA,__objc_catlist, regular, no_dead_strip";

  const char *classNameSection      = "__TEXT,__objc_classname, cstring_literals, regular, no_dead_strip";
  const char *classStubsSection     = "__DATA,__objc_stubs, regular, no_dead_strip";

  const char *constSection          = "__DATA,__objc_const, regular, no_dead_strip";
  const char *dataSection           = "__DATA,__objc_data, regular, no_dead_strip";

  const char *methodNameSection     = "__TEXT,__objc_methname, cstring_literals, regular, no_dead_strip";
  const char *methodTypeSection     = "__TEXT,__objc_methtype, regular, no_dead_strip";

  const char *classRefsSection      = "__DATA,__objc_classrefs, regular, no_dead_strip";
  const char *protoRefsSection      = "__DATA,__objc_protorefs, regular, no_dead_strip";
  const char *selectorRefsSection   = "__DATA,__objc_selrefs, regular, no_dead_strip";

private:
  llvm::Module &module;

  // Symbols that shouldn't be optimized away
  std::vector<llvm::Constant *> retainedSymbols;

  // Store the classes, protocols and new selectors.
  std::vector<ClassDeclaration *> classes;
  std::vector<InterfaceDeclaration *> protocols;
  std::vector<Identifier *> selectors;

  /// Cache for `__OBJC_METACLASS_$_`/`__OBJC_CLASS_$_` symbols.
  SymbolCache classSymbolTable;
  SymbolCache classRoSymbolTable;
  SymbolCache classRoNameTable;

  /// Cache for `_OBJC_CLASS_$_` symbols stored in `__objc_stubs`.
  /// NOTE: Stub classes have a different layout from normal classes
  /// And need to be instantiated with a call to the objective-c runtime.
  SymbolCache stubClassSymbolTable;

  /// Cache for `OBJC_CLASSLIST_REFERENCES_$_` symbols.
  SymbolCache classReferenceTable;

  /// Cache for `OBJC_PROTOLIST_REFERENCES_$_` symbols.
  SymbolCache protocolReferenceTable;

  /// Cache for `__OBJC_PROTOCOL_$_` symbols.
  SymbolCache protocolTable;
  SymbolCache protocolListTable;

  // Cache for methods.
  SymbolCache methodListTable;
  SymbolCache methodNameTable;
  SymbolCache methodTypeTable;

  // Cache for selectors.
  SymbolCache selectorTable;

  // Cache for instance variables.
  SymbolCache ivarTable;
  SymbolCache ivarListTable;
  SymbolCache ivarOffsetTable;

  // Generate name strings
  std::string getObjcTypeEncoding(Type *t);
  std::string getObjcClassRoSymbol(const ClassDeclaration& cd, bool meta);
  std::string getObjcClassSymbol(const ClassDeclaration& cd, bool meta);
  std::string getObjcMethodListSymbol(const ClassDeclaration& cd, bool meta);
  std::string getObjcProtoSymbol(const InterfaceDeclaration& iface);
  std::string getObjcIvarSymbol(const ClassDeclaration& cd, const VarDeclaration& var);
  std::string getObjcSymbolName(const char *dsymPrefix, const char *dsymName);

  // Generate name globals
  llvm::GlobalVariable *getClassRoName(const ClassDeclaration& cd);
  llvm::GlobalVariable *getProtocolName(const InterfaceDeclaration& cd);
  llvm::GlobalVariable *getMethodVarName(const llvm::StringRef& name);

  // Constant helpers
  llvm::Constant *constU32(uint32_t value);
  llvm::Constant *constU64(uint64_t value);
  llvm::Constant *constSizeT(size_t value);

  llvm::GlobalVariable *getEmptyCache();

  llvm::GlobalVariable *getMethodListFor(const ClassDeclaration& cd, bool meta, bool optional=false);

  llvm::GlobalVariable *getGlobal(llvm::Module& module, llvm::StringRef name, llvm::Type* type = nullptr);
  llvm::GlobalVariable *getGlobalWithBytes(llvm::Module& module, llvm::StringRef name, ConstantList packedContents);
  llvm::GlobalVariable *getCStringVar(const char *symbol, const llvm::StringRef &str, const char *section);

  void retain(llvm::Constant *sym);

  void genImageInfo();
  void retainSymbols();

  llvm::Constant *finalizeClasses();
  llvm::Constant *finalizeProtocols();
};
