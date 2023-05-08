// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: KVS_access.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_KVS_5faccess_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_KVS_5faccess_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3021000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3021012 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_KVS_5faccess_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_KVS_5faccess_2eproto {
  static const uint32_t offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_KVS_5faccess_2eproto;
namespace keyvaluestore {
class KV_pair;
struct KV_pairDefaultTypeInternal;
extern KV_pairDefaultTypeInternal _KV_pair_default_instance_;
class Key;
struct KeyDefaultTypeInternal;
extern KeyDefaultTypeInternal _Key_default_instance_;
class Value;
struct ValueDefaultTypeInternal;
extern ValueDefaultTypeInternal _Value_default_instance_;
}  // namespace keyvaluestore
PROTOBUF_NAMESPACE_OPEN
template<> ::keyvaluestore::KV_pair* Arena::CreateMaybeMessage<::keyvaluestore::KV_pair>(Arena*);
template<> ::keyvaluestore::Key* Arena::CreateMaybeMessage<::keyvaluestore::Key>(Arena*);
template<> ::keyvaluestore::Value* Arena::CreateMaybeMessage<::keyvaluestore::Value>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace keyvaluestore {

// ===================================================================

class Key final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:keyvaluestore.Key) */ {
 public:
  inline Key() : Key(nullptr) {}
  ~Key() override;
  explicit PROTOBUF_CONSTEXPR Key(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  Key(const Key& from);
  Key(Key&& from) noexcept
    : Key() {
    *this = ::std::move(from);
  }

  inline Key& operator=(const Key& from) {
    CopyFrom(from);
    return *this;
  }
  inline Key& operator=(Key&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const Key& default_instance() {
    return *internal_default_instance();
  }
  static inline const Key* internal_default_instance() {
    return reinterpret_cast<const Key*>(
               &_Key_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(Key& a, Key& b) {
    a.Swap(&b);
  }
  inline void Swap(Key* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(Key* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  Key* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<Key>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const Key& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const Key& from) {
    Key::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Key* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "keyvaluestore.Key";
  }
  protected:
  explicit Key(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kKeyFieldNumber = 1,
  };
  // string key = 1;
  void clear_key();
  const std::string& key() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_key(ArgT0&& arg0, ArgT... args);
  std::string* mutable_key();
  PROTOBUF_NODISCARD std::string* release_key();
  void set_allocated_key(std::string* key);
  private:
  const std::string& _internal_key() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_key(const std::string& value);
  std::string* _internal_mutable_key();
  public:

  // @@protoc_insertion_point(class_scope:keyvaluestore.Key)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr key_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_KVS_5faccess_2eproto;
};
// -------------------------------------------------------------------

class Value final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:keyvaluestore.Value) */ {
 public:
  inline Value() : Value(nullptr) {}
  ~Value() override;
  explicit PROTOBUF_CONSTEXPR Value(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  Value(const Value& from);
  Value(Value&& from) noexcept
    : Value() {
    *this = ::std::move(from);
  }

  inline Value& operator=(const Value& from) {
    CopyFrom(from);
    return *this;
  }
  inline Value& operator=(Value&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const Value& default_instance() {
    return *internal_default_instance();
  }
  static inline const Value* internal_default_instance() {
    return reinterpret_cast<const Value*>(
               &_Value_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  friend void swap(Value& a, Value& b) {
    a.Swap(&b);
  }
  inline void Swap(Value* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(Value* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  Value* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<Value>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const Value& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const Value& from) {
    Value::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(Value* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "keyvaluestore.Value";
  }
  protected:
  explicit Value(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kValueFieldNumber = 1,
  };
  // string value = 1;
  void clear_value();
  const std::string& value() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_value(ArgT0&& arg0, ArgT... args);
  std::string* mutable_value();
  PROTOBUF_NODISCARD std::string* release_value();
  void set_allocated_value(std::string* value);
  private:
  const std::string& _internal_value() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_value(const std::string& value);
  std::string* _internal_mutable_value();
  public:

  // @@protoc_insertion_point(class_scope:keyvaluestore.Value)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr value_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_KVS_5faccess_2eproto;
};
// -------------------------------------------------------------------

class KV_pair final :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:keyvaluestore.KV_pair) */ {
 public:
  inline KV_pair() : KV_pair(nullptr) {}
  ~KV_pair() override;
  explicit PROTOBUF_CONSTEXPR KV_pair(::PROTOBUF_NAMESPACE_ID::internal::ConstantInitialized);

  KV_pair(const KV_pair& from);
  KV_pair(KV_pair&& from) noexcept
    : KV_pair() {
    *this = ::std::move(from);
  }

  inline KV_pair& operator=(const KV_pair& from) {
    CopyFrom(from);
    return *this;
  }
  inline KV_pair& operator=(KV_pair&& from) noexcept {
    if (this == &from) return *this;
    if (GetOwningArena() == from.GetOwningArena()
  #ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetOwningArena() != nullptr
  #endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const KV_pair& default_instance() {
    return *internal_default_instance();
  }
  static inline const KV_pair* internal_default_instance() {
    return reinterpret_cast<const KV_pair*>(
               &_KV_pair_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    2;

  friend void swap(KV_pair& a, KV_pair& b) {
    a.Swap(&b);
  }
  inline void Swap(KV_pair* other) {
    if (other == this) return;
  #ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() != nullptr &&
        GetOwningArena() == other->GetOwningArena()) {
   #else  // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetOwningArena() == other->GetOwningArena()) {
  #endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(KV_pair* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetOwningArena() == other->GetOwningArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  KV_pair* New(::PROTOBUF_NAMESPACE_ID::Arena* arena = nullptr) const final {
    return CreateMaybeMessage<KV_pair>(arena);
  }
  using ::PROTOBUF_NAMESPACE_ID::Message::CopyFrom;
  void CopyFrom(const KV_pair& from);
  using ::PROTOBUF_NAMESPACE_ID::Message::MergeFrom;
  void MergeFrom( const KV_pair& from) {
    KV_pair::MergeImpl(*this, from);
  }
  private:
  static void MergeImpl(::PROTOBUF_NAMESPACE_ID::Message& to_msg, const ::PROTOBUF_NAMESPACE_ID::Message& from_msg);
  public:
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  uint8_t* _InternalSerialize(
      uint8_t* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::PROTOBUF_NAMESPACE_ID::Arena* arena, bool is_message_owned);
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(KV_pair* other);

  private:
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "keyvaluestore.KV_pair";
  }
  protected:
  explicit KV_pair(::PROTOBUF_NAMESPACE_ID::Arena* arena,
                       bool is_message_owned = false);
  public:

  static const ClassData _class_data_;
  const ::PROTOBUF_NAMESPACE_ID::Message::ClassData*GetClassData() const final;

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kKeyFieldNumber = 1,
    kValueFieldNumber = 2,
  };
  // string key = 1;
  void clear_key();
  const std::string& key() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_key(ArgT0&& arg0, ArgT... args);
  std::string* mutable_key();
  PROTOBUF_NODISCARD std::string* release_key();
  void set_allocated_key(std::string* key);
  private:
  const std::string& _internal_key() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_key(const std::string& value);
  std::string* _internal_mutable_key();
  public:

  // string value = 2;
  void clear_value();
  const std::string& value() const;
  template <typename ArgT0 = const std::string&, typename... ArgT>
  void set_value(ArgT0&& arg0, ArgT... args);
  std::string* mutable_value();
  PROTOBUF_NODISCARD std::string* release_value();
  void set_allocated_value(std::string* value);
  private:
  const std::string& _internal_value() const;
  inline PROTOBUF_ALWAYS_INLINE void _internal_set_value(const std::string& value);
  std::string* _internal_mutable_value();
  public:

  // @@protoc_insertion_point(class_scope:keyvaluestore.KV_pair)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  struct Impl_ {
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr key_;
    ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr value_;
    mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_KVS_5faccess_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// Key

// string key = 1;
inline void Key::clear_key() {
  _impl_.key_.ClearToEmpty();
}
inline const std::string& Key::key() const {
  // @@protoc_insertion_point(field_get:keyvaluestore.Key.key)
  return _internal_key();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void Key::set_key(ArgT0&& arg0, ArgT... args) {
 
 _impl_.key_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:keyvaluestore.Key.key)
}
inline std::string* Key::mutable_key() {
  std::string* _s = _internal_mutable_key();
  // @@protoc_insertion_point(field_mutable:keyvaluestore.Key.key)
  return _s;
}
inline const std::string& Key::_internal_key() const {
  return _impl_.key_.Get();
}
inline void Key::_internal_set_key(const std::string& value) {
  
  _impl_.key_.Set(value, GetArenaForAllocation());
}
inline std::string* Key::_internal_mutable_key() {
  
  return _impl_.key_.Mutable(GetArenaForAllocation());
}
inline std::string* Key::release_key() {
  // @@protoc_insertion_point(field_release:keyvaluestore.Key.key)
  return _impl_.key_.Release();
}
inline void Key::set_allocated_key(std::string* key) {
  if (key != nullptr) {
    
  } else {
    
  }
  _impl_.key_.SetAllocated(key, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.key_.IsDefault()) {
    _impl_.key_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:keyvaluestore.Key.key)
}

// -------------------------------------------------------------------

// Value

// string value = 1;
inline void Value::clear_value() {
  _impl_.value_.ClearToEmpty();
}
inline const std::string& Value::value() const {
  // @@protoc_insertion_point(field_get:keyvaluestore.Value.value)
  return _internal_value();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void Value::set_value(ArgT0&& arg0, ArgT... args) {
 
 _impl_.value_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:keyvaluestore.Value.value)
}
inline std::string* Value::mutable_value() {
  std::string* _s = _internal_mutable_value();
  // @@protoc_insertion_point(field_mutable:keyvaluestore.Value.value)
  return _s;
}
inline const std::string& Value::_internal_value() const {
  return _impl_.value_.Get();
}
inline void Value::_internal_set_value(const std::string& value) {
  
  _impl_.value_.Set(value, GetArenaForAllocation());
}
inline std::string* Value::_internal_mutable_value() {
  
  return _impl_.value_.Mutable(GetArenaForAllocation());
}
inline std::string* Value::release_value() {
  // @@protoc_insertion_point(field_release:keyvaluestore.Value.value)
  return _impl_.value_.Release();
}
inline void Value::set_allocated_value(std::string* value) {
  if (value != nullptr) {
    
  } else {
    
  }
  _impl_.value_.SetAllocated(value, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.value_.IsDefault()) {
    _impl_.value_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:keyvaluestore.Value.value)
}

// -------------------------------------------------------------------

// KV_pair

// string key = 1;
inline void KV_pair::clear_key() {
  _impl_.key_.ClearToEmpty();
}
inline const std::string& KV_pair::key() const {
  // @@protoc_insertion_point(field_get:keyvaluestore.KV_pair.key)
  return _internal_key();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void KV_pair::set_key(ArgT0&& arg0, ArgT... args) {
 
 _impl_.key_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:keyvaluestore.KV_pair.key)
}
inline std::string* KV_pair::mutable_key() {
  std::string* _s = _internal_mutable_key();
  // @@protoc_insertion_point(field_mutable:keyvaluestore.KV_pair.key)
  return _s;
}
inline const std::string& KV_pair::_internal_key() const {
  return _impl_.key_.Get();
}
inline void KV_pair::_internal_set_key(const std::string& value) {
  
  _impl_.key_.Set(value, GetArenaForAllocation());
}
inline std::string* KV_pair::_internal_mutable_key() {
  
  return _impl_.key_.Mutable(GetArenaForAllocation());
}
inline std::string* KV_pair::release_key() {
  // @@protoc_insertion_point(field_release:keyvaluestore.KV_pair.key)
  return _impl_.key_.Release();
}
inline void KV_pair::set_allocated_key(std::string* key) {
  if (key != nullptr) {
    
  } else {
    
  }
  _impl_.key_.SetAllocated(key, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.key_.IsDefault()) {
    _impl_.key_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:keyvaluestore.KV_pair.key)
}

// string value = 2;
inline void KV_pair::clear_value() {
  _impl_.value_.ClearToEmpty();
}
inline const std::string& KV_pair::value() const {
  // @@protoc_insertion_point(field_get:keyvaluestore.KV_pair.value)
  return _internal_value();
}
template <typename ArgT0, typename... ArgT>
inline PROTOBUF_ALWAYS_INLINE
void KV_pair::set_value(ArgT0&& arg0, ArgT... args) {
 
 _impl_.value_.Set(static_cast<ArgT0 &&>(arg0), args..., GetArenaForAllocation());
  // @@protoc_insertion_point(field_set:keyvaluestore.KV_pair.value)
}
inline std::string* KV_pair::mutable_value() {
  std::string* _s = _internal_mutable_value();
  // @@protoc_insertion_point(field_mutable:keyvaluestore.KV_pair.value)
  return _s;
}
inline const std::string& KV_pair::_internal_value() const {
  return _impl_.value_.Get();
}
inline void KV_pair::_internal_set_value(const std::string& value) {
  
  _impl_.value_.Set(value, GetArenaForAllocation());
}
inline std::string* KV_pair::_internal_mutable_value() {
  
  return _impl_.value_.Mutable(GetArenaForAllocation());
}
inline std::string* KV_pair::release_value() {
  // @@protoc_insertion_point(field_release:keyvaluestore.KV_pair.value)
  return _impl_.value_.Release();
}
inline void KV_pair::set_allocated_value(std::string* value) {
  if (value != nullptr) {
    
  } else {
    
  }
  _impl_.value_.SetAllocated(value, GetArenaForAllocation());
#ifdef PROTOBUF_FORCE_COPY_DEFAULT_STRING
  if (_impl_.value_.IsDefault()) {
    _impl_.value_.Set("", GetArenaForAllocation());
  }
#endif // PROTOBUF_FORCE_COPY_DEFAULT_STRING
  // @@protoc_insertion_point(field_set_allocated:keyvaluestore.KV_pair.value)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace keyvaluestore

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_KVS_5faccess_2eproto
