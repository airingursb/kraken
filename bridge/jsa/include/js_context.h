/*
 * Copyright (C) 2019 Alibaba Inc. All rights reserved.
 * Author: Kraken Team.
 */

#ifndef JSA_JSCONTEXT_H
#define JSA_JSCONTEXT_H

#include "macros.h"
#include <functional>
#include <memory>
#include <vector>

namespace alibaba {
namespace jsa {

class JSContext;
class Pointer;
class PropNameID;
class Symbol;
class String;
class Object;
class WeakObject;
class Array;
class ArrayBuffer;
class Function;
class Value;
class Instrumentation;
class Scope;
class JSAException;
class JSError;

/// A function which has this type can be registered as a function
/// callable from JavaScript using Function::createFromHostFunction().
/// When the function is called, args will point to the arguments, and
/// count will indicate how many arguments are passed.  The function
/// can return a Value to the caller, or throw an exception.  If a C++
/// exception is thrown, a JS Error will be created and thrown into
/// JS; if the C++ exception extends std::exception, the Error's
/// message will be whatever what() returns. Note that it is undefined whether
/// HostFunctions may or may not be called in strict mode; that is `thisVal`
/// can be any value - it will not necessarily be coerced to an object or
/// or set to the global object.
using HostFunctionType = std::function<Value(
    JSContext &rt, const Value &thisVal, const Value *args, size_t count)>;

/// An object which implements this interface can be registered as an
/// Object with the JS runtime.
class HostObject {
public:
  // The C++ object's dtor will be called when the GC finalizes this
  // object.  (This may be as late as when the JSContext is shut down.)
  // You have no control over which thread it is called on.  This will
  // be called from inside the GC, so it is unsafe to do any VM
  // operations which require a JSContext&.  Derived classes' dtors
  // should also avoid doing anything expensive.  Calling the dtor on
  // a jsa object is explicitly ok.  If you want to do JS operations,
  // or any nontrivial work, you should add it to a work queue, and
  // manage it externally.
  virtual ~HostObject();

  // When JS wants a property with a given name from the HostObject,
  // it will call this method.  If it throws an exception, the call
  // will throw a JS \c Error object. By default this returns undefined.
  // \return the value for the property.
  virtual Value get(JSContext &, const PropNameID &name);

  // When JS wants to set a property with a given name on the HostObject,
  // it will call this method. If it throws an exception, the call will
  // throw a JS \c Error object. By default this throws a type error exception
  // mimicking the behavior of a frozen object in strict mode.
  virtual void set(JSContext &, const PropNameID &name, const Value &value);

  // When JS wants a list of property names for the HostObject, it will
  // call this method. If it throws an exception, the call will thow a
  // JS \c Error object. The default implementation returns empty vector.
  virtual std::vector<PropNameID> getPropertyNames(JSContext &rt);
};

class ThreadScope {
public:
  using Task = void (*)(void *);
  ThreadScope() = default;
  virtual ~ThreadScope() = default;

  virtual void postToUIThread(Task task, void *data) = 0;
};

/// Represents a JS runtime.  Movable, but not copyable.  Note that
/// this object may not be thread-aware, but cannot be used safely from
/// multiple threads at once.  The application is responsible for
/// ensuring that it is used safely.  This could mean using the
/// Runtime from a single thread, using a mutex, doing all work on a
/// serial queue, etc.  This restriction applies to the methods of
/// this class, and any method in the API which take a Runtime& as an
/// argument.  Destructors (all but ~Scope), operators, or other methods
/// which do not take Runtime& as an argument are safe to call from any
/// thread, but it is still forbidden to make write operations on a single
/// instance of any class from more than one thread.  In addition, to
/// make shutdown safe, destruction of objects associated with the Runtime
/// must be destroyed before the Runtime is destroyed, or from the
/// destructor of a managed HostObject or HostFunction.  Informally, this
/// means that the main source of unsafe behavior is to hold a jsa object
/// in a non-Runtime-managed object, and not clean it up before the Runtime
/// is shut down.  If your lifecycle is such that avoiding this is hard,
/// you will probably need to do use your own locks.
class JSContext {
public:
  virtual ~JSContext();

  /// Evaluates the given JavaScript \c buffer.  \c sourceURL is used
  /// to annotate the stack trace if there is an exception.  The
  /// contents may be utf8-encoded JS source code, or binary bytcode
  /// whose format is specific to the implementation.  If the input
  /// format is unknown, or evaluation causes an error, a JSAException
  /// will be thrown.
  /// Note this function should ONLY be used when there isn't another means
  /// through the JSA API. For example, it will be much slower to use this to
  /// call a global function than using the JSA APIs to read the function
  /// property from the global object and then calling it explicitly.
  virtual Value evaluateJavaScript(const char *code,
                                   const std::string &sourceURL,
                                   int startLine) = 0;

  /// \return the global object
  virtual Object global() = 0;

  /// \return a short printable description of the instance.  This
  /// should only be used by logging, debugging, and other
  /// developer-facing callers.
  virtual std::string description() = 0;

  /// \return whether or not the underlying runtime supports debugging via the
  /// Chrome remote debugging protocol.
  ///
  /// NOTE: the API for determining whether a runtime is debuggable and
  /// registering a runtime with the debugger is still in flux, so please don't
  /// use this API unless you know what you're doing.
  virtual bool isInspectable() = 0;

  /// \return an interface to extract metrics from this \c Runtime.  The default
  /// implementation of this function returns an \c Instrumentation instance
  /// which returns no metrics.
  virtual Instrumentation &instrumentation();

  /// \return JS Engine's actual global object,
  /// in most of the time, you should use global() method instead of this.
  virtual void *globalImpl() = 0;

  void bindThreadScope(std::unique_ptr<ThreadScope> threadScope) {
    thread_scope_ptr_ = std::move(threadScope);
  }

  ThreadScope *threadScope() const {
    if (thread_scope_ptr_ == nullptr) {
      return nullptr;
    }
    return thread_scope_ptr_.get();
  }

protected:
  friend class Pointer;
  friend class PropNameID;
  friend class Symbol;
  friend class String;
  friend class Object;
  friend class WeakObject;
  friend class Array;
  friend class ArrayBuffer;
  friend class Function;
  friend class Value;
  friend class Scope;
  friend class JSError;

  // Potential optimization: avoid the cloneFoo() virtual dispatch,
  // and instead just fix the number of fields, and copy them, since
  // in practice they are trivially copyable.  Sufficient use of
  // rvalue arguments/methods would also reduce the number of clones.

  // 代表具体JS运行时实际的类型(String|Object|Symbol)接口。
  struct PointerValue {

    // 释放
    // jsa::Point在析构时会调用此方法
    virtual void invalidate() = 0;

  protected:
    virtual ~PointerValue() = default;
  };

  virtual PointerValue *cloneSymbol(const JSContext::PointerValue *pv) = 0;
  virtual PointerValue *cloneString(const JSContext::PointerValue *pv) = 0;
  virtual PointerValue *cloneObject(const JSContext::PointerValue *pv) = 0;
  virtual PointerValue *clonePropNameID(const JSContext::PointerValue *pv) = 0;

  virtual PropNameID createPropNameIDFromAscii(const char *str,
                                               size_t length) = 0;
  virtual PropNameID createPropNameIDFromUtf8(const uint8_t *utf8,
                                              size_t length) = 0;
  virtual PropNameID createPropNameIDFromString(const String &str) = 0;
  virtual std::string utf8(const PropNameID &) = 0;
  virtual bool compare(const PropNameID &, const PropNameID &) = 0;

  virtual std::string symbolToString(const Symbol &) = 0;

  virtual String createStringFromAscii(const char *str, size_t length) = 0;
  virtual String createStringFromUtf8(const uint8_t *utf8, size_t length) = 0;
  virtual std::string utf8(const String &) = 0;

  virtual Object createObject() = 0;
  virtual Object createObject(std::shared_ptr<HostObject> ho) = 0;
  virtual std::shared_ptr<HostObject> getHostObject(const jsa::Object &) = 0;
  virtual HostFunctionType &getHostFunction(const jsa::Function &) = 0;

  virtual Value getProperty(const Object &, const PropNameID &name) = 0;
  virtual Value getProperty(const Object &, const String &name) = 0;
  virtual bool hasProperty(const Object &, const PropNameID &name) = 0;
  virtual bool hasProperty(const Object &, const String &name) = 0;
  virtual void setPropertyValue(Object &, const PropNameID &name,
                                const Value &value) = 0;
  virtual void setPropertyValue(Object &, const String &name,
                                const Value &value) = 0;

  virtual bool isArray(const Object &) const = 0;
  virtual bool isArrayBuffer(const Object &) const = 0;
  virtual bool isFunction(const Object &) const = 0;
  virtual bool isHostObject(const jsa::Object &) const = 0;
  virtual bool isHostFunction(const jsa::Function &) const = 0;
  virtual Array getPropertyNames(const Object &) = 0;

  virtual WeakObject createWeakObject(const Object &) = 0;
  virtual Value lockWeakObject(const WeakObject &) = 0;

  virtual Array createArray(size_t length) = 0;
  virtual size_t size(const Array &) = 0;
  virtual size_t size(const ArrayBuffer &) = 0;
  virtual uint8_t *data(const ArrayBuffer &) = 0;
  virtual Value getValueAtIndex(const Array &, size_t i) = 0;
  virtual void setValueAtIndexImpl(Array &, size_t i, const Value &value) = 0;

  virtual Function createFunctionFromHostFunction(const PropNameID &name,
                                                  unsigned int paramCount,
                                                  HostFunctionType func) = 0;
  virtual Value call(const Function &, const Value &jsThis, const Value *args,
                     size_t count) = 0;
  virtual Value callAsConstructor(const Function &, const Value *args,
                                  size_t count) = 0;

  // Private data for managing scopes.
  struct ScopeState;
  virtual ScopeState *pushScope();
  virtual void popScope(ScopeState *);

  virtual bool strictEquals(const Symbol &a, const Symbol &b) const = 0;
  virtual bool strictEquals(const String &a, const String &b) const = 0;
  virtual bool strictEquals(const Object &a, const Object &b) const = 0;

  virtual bool instanceOf(const Object &o, const Function &f) = 0;

  // These exist so derived classes can access the private parts of
  // Value, Symbol, String, and Object, which are all friends of JSContext.
  template <typename T> static T make(PointerValue *pv) { return T(pv); }
  static const PointerValue *getPointerValue(const Pointer &pointer);
  static const PointerValue *getPointerValue(const Value &value);

  template <typename Plain, typename Base> friend class RuntimeDecorator;

private:
  std::unique_ptr<ThreadScope> thread_scope_ptr_;
};

/// Not movable and not copyable RAII marker advising the underlying
/// JavaScript VM to track resources allocated since creation until
/// destruction so that they can be recycled eagerly when the Scope
/// goes out of scope instead of floating in the air until the next
/// garbage collection or any other delayed release occurs.
///
/// This API should be treated only as advice, implementations can
/// choose to ignore the fact that Scopes are created or destroyed.
///
/// This class is an exception to the rule allowing destructors to be
/// called without proper synchronization (see Runtime documentation).
/// The whole point of this class is to enable all sorts of clean ups
/// when the destructor is called and this proper synchronization is
/// required at that time.
///
/// Instances of this class are intended to be created as automatic stack
/// variables in which case destructor calls don't require any additional
/// locking, provided that the lock (if any) is managed with RAII helpers.
class Scope {
public:
  explicit Scope(JSContext &rt) : rt_(rt), prv_(rt.pushScope()) {}
  ~Scope() { rt_.popScope(prv_); };

  Scope(const Scope &) = delete;
  Scope(Scope &&) = delete;

  Scope &operator=(const Scope &) = delete;
  Scope &operator=(Scope &&) = delete;

  template <typename F>
  static auto callInNewScope(JSContext &rt, F f) -> decltype(f()) {
    Scope s(rt);
    return f();
  }

private:
  JSContext &rt_;
  JSContext::ScopeState *prv_;
};

} // namespace jsa
} // namespace alibaba

#endif // JSA_JSCONTEXT_H