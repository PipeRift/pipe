// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Templates.h"
#include "Pipe/Reflect/EnumType.h"
#include "Pipe/Reflect/ReflectionTraits.h"
#include "Pipe/Reflect/Type.h"
#include "Pipe/Reflect/TypeFlags.h"
#include "PipeArrays.h"
#include "PipeSerializeFwd.h"

#include <utility>


struct yyjson_doc;
struct yyjson_mut_doc;
struct yyjson_val;
struct yyjson_mut_val;


namespace p
{
	namespace details
	{
		template<typename T>
		struct HasRead : std::false_type
		{};
		template<typename T>
		requires IsVoid<decltype(Read(std::declval<struct Reader&>(), std::declval<T&>()))>
		struct HasRead<T> : std::true_type
		{};

		template<typename T>
		struct HasWrite : std::false_type
		{};
		template<typename T>
		requires IsVoid<decltype(Write(std::declval<struct Writer&>(), std::declval<const T&>()))>
		struct HasWrite<T> : std::true_type
		{};
	}    // namespace details


	template<typename T>
	static constexpr bool Readable = details::template HasRead<T>::value;
	template<typename T>
	static constexpr bool Writable = details::template HasWrite<T>::value;


	enum WriteFlags : sizet
	{
		WriteFlags_None              = 0,
		WriteFlags_CacheStringKeys   = 1 << 0,
		WriteFlags_CacheStringValues = 1 << 1,
		WriteFlags_CacheStrings      = WriteFlags_CacheStringKeys | WriteFlags_CacheStringValues
	};


#pragma region Reader
	struct PIPE_API Reader
	{
		friend IFormatReader;
		IFormatReader* formatReader = nullptr;


	private:
		// Initialize child classes using the copy constructor
		Reader() = default;

	public:
		explicit Reader(const Reader&)   = default;
		Reader& operator=(const Reader&) = default;
		virtual ~Reader() {}


		/**
		 * Marks current scope as an Object.
		 * See EnterNext(name) & Next(name, value)
		 */
		void BeginObject();

		/**
		 * Enters the scope of a key in an object.
		 * To make an object scope, see 'BeginObject()'
		 * Complexity: Ordered access O(1), Inverse order O(n)
		 * @return true if inside an object scope and the key is found.
		 */
		bool EnterNext(StringView name);

		/**
		 * Finds and reads a value at key "name" of an object.
		 * Complexity: Ordered access O(1), Inverse order O(n)
		 * This function won't do anything on array or uninitialized scopes
		 */
		template<typename T>
		void Next(StringView name, T& val)
		{
			if (EnterNext(name))
			{
				Serialize(val);
				Leave();
			}
		}

		/**
		 * Marks current scope as an Array.
		 * See EnterNext() & Next(value)
		 * @param size of the array being read
		 */
		void BeginArray(u32& size);

		/**
		 * Enters the scope of the next element of an array.
		 * To make an object scope, see 'BeginArray()'
		 * Complexity: O(1)
		 * @return true if inside an array scope and num elements is not exceeded.
		 */
		bool EnterNext();

		/**
		 * Reads a type from the next element of an array.
		 * Complexity: O(1)
		 * This function won't do anything on object or uninitialized scopes
		 */
		template<typename T>
		void Next(T& val)
		{
			if (EnterNext())
			{
				Serialize(val);
				Leave();
			}
		}

		// Reads a type from the current scope
		template<typename T>
		void Serialize(T& val)
		{
			static_assert(Readable<T>,
			    "Type must be readable! No valid read function found. E.g: 'Read(Reader& w, T& "
			    "value)'");
			if constexpr (Readable<T>)
			{
				Read(*this, val);
			}
		}

		void Leave();

		bool IsObject();
		bool IsArray();

		constexpr bool IsReading()
		{
			return true;
		}
		constexpr bool IsWriting()
		{
			return false;
		}

		inline IFormatReader& GetFormat()
		{
			return *formatReader;
		}
	};

	// Format reads
	PIPE_API void Read(Reader& r, bool& val);
	PIPE_API void Read(Reader& r, i8& val);
	PIPE_API void Read(Reader& r, u8& val);
	PIPE_API void Read(Reader& r, i16& val);
	PIPE_API void Read(Reader& r, u16& val);
	PIPE_API void Read(Reader& r, i32& val);
	PIPE_API void Read(Reader& r, u32& val);
	PIPE_API void Read(Reader& r, i64& val);
	PIPE_API void Read(Reader& r, u64& val);
	PIPE_API void Read(Reader& r, float& val);
	PIPE_API void Read(Reader& r, double& val);
	PIPE_API void Read(Reader& r, StringView& val);

	// Pipe types reads
	PIPE_API void Read(Reader& r, Type*& val);
	PIPE_API void Read(Reader& r, TypeId& val);

	template<typename T1, typename T2>
	void Read(Reader& r, TPair<T1, T2>& val)
	{
		r.BeginObject();
		r.Next("first", val.first);
		r.Next("second", val.second);
	}

	template<typename T>
	void Read(Reader& r, T& val)
	    requires(bool(TFlags<T>::HasMemberSerialize && !TFlags<T>::HasSingleSerialize))
	{
		val.Read(r);
	}

	template<typename T>
	void Read(Reader& r, T& val) requires(IsArray<T>())
	{
		u32 size;
		r.BeginArray(size);
		val.Resize(size);
		for (u32 i = 0; i < size; ++i)
		{
			r.Next(val[i]);
		}
	}

	template<typename T>
	void Read(Reader& r, T& val) requires(Enum<T>)
	{
		if constexpr (GetEnumSize<T>() > 0)
		{
			StringView typeStr;
			r.Serialize(typeStr);
			if (std::optional<T> value = GetEnumValue<T>(typeStr))
			{
				val = value.value();
			}
		}
	}
#pragma endregion Reader


#pragma region Writer
	struct PIPE_API Writer
	{
		friend IFormatWriter;
		IFormatWriter* formatWriter = nullptr;


	private:
		Writer() = default;

	public:
		explicit Writer(const Writer&)   = default;
		Writer& operator=(const Writer&) = default;
		virtual ~Writer() {}

		/**
		 * Marks current scope as an Object.
		 * See EnterNext(name) & Next(name, value)
		 */
		void BeginObject();

		/**
		 * Enters the scope of a key in an object.
		 * To make object scope, see 'BeginObject()'
		 * @return true if inside an object scope and the key is found.
		 */
		bool EnterNext(StringView name);

		/**
		 * Finds and writes a type at key "name" of an object.
		 * This function won't do anything on array or uninitialized scopes
		 */
		template<typename T>
		void Next(StringView name, const T& val)
		{
			if (EnterNext(name))
			{
				Serialize(val);
				Leave();
			}
		}

		/**
		 * Marks current scope as an Array.
		 * See EnterNext() & Next(value)
		 * @param size of the array being read
		 */
		void BeginArray(u32 size);

		/**
		 * Enters the scope of the next element of an array.
		 * To make an object scope, see 'BeginArray()'
		 * Complexity: O(1)
		 * @return true if inside an array scope and num elements is not exceeded.
		 */
		bool EnterNext();

		/**
		 * Reads a type from the next element of an array.
		 * Complexity: O(1)
		 * This function won't do anything on object or uninitialized scopes
		 */
		template<typename T>
		void Next(const T& val)
		{
			if (EnterNext())
			{
				Serialize(val);
				Leave();
			}
		}


		// Write a type into the current scope
		template<typename T>
		void Serialize(const T& val)
		{
			static_assert(Writable<T>,
			    "Type must be writable! No valid write function found. E.g: 'Write(Writer& w, "
			    "const T& value)'");
			if constexpr (Writable<T>)
			{
				Write(*this, val);
			}
		}

		void Leave();

		static constexpr bool IsReading()
		{
			return false;
		}
		static constexpr bool IsWriting()
		{
			return true;
		}

		void PushAddFlags(WriteFlags flags);
		void PushRemoveFlags(WriteFlags flags);
		void PopFlags();

		inline IFormatWriter& GetFormat()
		{
			return *formatWriter;
		}
	};

	// Format writes
	PIPE_API void Write(Writer& w, bool val);
	PIPE_API void Write(Writer& w, i8 val);
	PIPE_API void Write(Writer& w, u8 val);
	PIPE_API void Write(Writer& w, i16 val);
	PIPE_API void Write(Writer& w, u16 val);
	PIPE_API void Write(Writer& w, i32 val);
	PIPE_API void Write(Writer& w, u32 val);
	PIPE_API void Write(Writer& w, i64 val);
	PIPE_API void Write(Writer& w, u64 val);
	PIPE_API void Write(Writer& w, float val);
	PIPE_API void Write(Writer& w, double val);
	PIPE_API void Write(Writer& w, StringView val);

	// Pipe types writes
	PIPE_API void Write(Writer& w, Type* val);
	PIPE_API void Write(Writer& w, TypeId val);

	template<typename T1, typename T2>
	void Write(Writer& w, TPair<T1, T2>& val)
	{
		w.BeginObject();
		w.Next("first", val.first);
		w.Next("second", val.second);
	}

	template<typename T>
	void Write(Writer& w, const T& val)
	    requires(bool(TFlags<T>::HasMemberSerialize && !TFlags<T>::HasSingleSerialize))
	{
		val.Write(w);
	}

	template<typename T>
	void Write(Writer& w, const T& val) requires(IsArrayView<T>())
	{
		u32 size = val.Size();
		w.BeginArray(size);
		for (u32 i = 0; i < size; ++i)
		{
			w.Next(val[i]);
		}
	}

	template<typename T>
	void Write(Writer& w, T& val) requires Enum<T>
	{
		if constexpr (GetEnumSize<T>() > 0)
		{
			// Might not be necessary to cache string since enum name is static
			w.PushAddFlags(WriteFlags_CacheStringValues);
			w.Serialize(GetEnumName(val));
			w.PopFlags();
		}
	}
#pragma endregion Writer


#pragma region ReadWriter
	struct PIPE_API ReadWriter
	{
	private:
		Reader* reader;
		Writer* writer;


	public:
		ReadWriter(Reader& reader) : reader{&reader}, writer{nullptr} {}
		ReadWriter(Writer& writer) : reader{nullptr}, writer{&writer} {}
		ReadWriter(const ReadWriter& other)            = default;
		ReadWriter& operator=(const ReadWriter& other) = default;

		/**
		 * Starts the deserialization of an scope as an object.
		 */
		void BeginObject();

		/**
		 * Enters the scope of a key in an object.
		 * This function will fail on array scopes
		 */
		bool EnterNext(StringView name);

		/**
		 * Deserializes a value from an object key
		 * This function will fail on array scopes
		 */
		template<typename T>
		void Next(StringView name, T& val)
		{
			if (IsWriting())
			{
				writer->Next(name, val);
			}
			else
			{
				reader->Next(name, val);
			}
		}


		/**
		 * Starts the deserialization of an scope as an array.
		 * @param size of the array being read
		 */
		void BeginArray(u32& size);

		/**
		 * Enters the scope of the next element of an array.
		 * This function will fail on object scopes
		 */
		bool EnterNext();

		/**
		 * Deserializes a value from the next element of an array.
		 * This function will fail on object scopes
		 */
		template<typename T>
		void Next(T& val)
		{
			if (IsWriting())
			{
				// writeContext->Next(val);
			}
			else
			{
				reader->Next(val);
			}
		}

		// Reads or Writes a type from the current scope
		template<typename T>
		void Serialize(T& val)
		{
			if (IsWriting())
			{
				writer->Serialize(const_cast<const T&>(val));
			}
			else
			{
				reader->Serialize(val);
			}
		}

		void Leave();

		bool IsReading() const
		{
			return reader != nullptr;
		}
		bool IsWriting() const
		{
			return writer != nullptr;
		}

		Reader& GetRead() const
		{
			P_Check(IsReading());
			return *reader;
		}

		Writer& GetWrite() const
		{
			P_Check(IsWriting());
			return *writer;
		}

		void PushAddFlags(WriteFlags flags);
		void PushRemoveFlags(WriteFlags flags);
		void PopFlags();
	};


	// Types can be marked as single serialize, so that Serialize() will be called instead of Read
	// and Write
	template<typename T>
	void Read(Reader& ct, T& val)
	    requires(bool(TFlags<T>::HasSingleSerialize&& TFlags<T>::HasMemberSerialize))
	{
		ReadWriter commonContext{ct};
		val.Serialize(commonContext);
	}
	template<typename T>
	void Read(Reader& ct, T& val)
	    requires(bool(TFlags<T>::HasSingleSerialize && !TFlags<T>::HasMemberSerialize))
	{
		ReadWriter commonContext{ct};
		Serialize(commonContext, val);
	}

	// Types can be marked as single serialize, so that Serialize() will be called instead of Read
	// and Write
	template<typename T>
	void Write(Writer& ct, const T& val)
	    requires(bool(TFlags<T>::HasSingleSerialize&& TFlags<T>::HasMemberSerialize))
	{
		ReadWriter commonContext{ct};
		const_cast<T&>(val).Serialize(commonContext);
	}
	template<typename T>
	void Write(Writer& ct, const T& val)
	    requires(bool(TFlags<T>::HasSingleSerialize && !TFlags<T>::HasMemberSerialize))
	{
		ReadWriter commonContext{ct};
		Serialize(commonContext, const_cast<T&>(val));
	}
#pragma endregion ReadWriter


#pragma region Format Interface
	struct PIPE_API IFormatReader
	{
	private:
		Reader reader{};


	public:
		IFormatReader()
		{
			reader.formatReader = this;
		}

		virtual ~IFormatReader()                = default;
		virtual void BeginObject()              = 0;
		virtual void BeginArray(u32& size)      = 0;
		virtual bool EnterNext(StringView name) = 0;
		virtual bool EnterNext()                = 0;
		virtual void Leave()                    = 0;
		virtual void Read(bool& val)            = 0;
		virtual void Read(i8& val)              = 0;
		virtual void Read(u8& val)              = 0;
		virtual void Read(i16& val)             = 0;
		virtual void Read(u16& val)             = 0;
		virtual void Read(i32& val)             = 0;
		virtual void Read(u32& val)             = 0;
		virtual void Read(i64& val)             = 0;
		virtual void Read(u64& val)             = 0;
		virtual void Read(float& val)           = 0;
		virtual void Read(double& val)          = 0;
		virtual void Read(StringView& val)      = 0;
		virtual bool IsObject() const           = 0;
		virtual bool IsArray() const            = 0;
		virtual bool IsValid() const            = 0;

		Reader& GetReader()
		{
			return reader;
		}
		const Reader& GetReader() const
		{
			return reader;
		}
		operator Reader&()
		{
			return GetReader();
		}
	};


	struct PIPE_API IFormatWriter
	{
	private:
		Writer writer{};


		sizet activeFlags = WriteFlags_None;
		TArray<sizet> flagStack;

	public:
		IFormatWriter()
		{
			writer.formatWriter = this;
		}

		virtual ~IFormatWriter()                = default;
		virtual void BeginObject()              = 0;
		virtual void BeginArray(u32 size)       = 0;
		virtual bool EnterNext(StringView name) = 0;
		virtual bool EnterNext()                = 0;
		virtual void Leave()                    = 0;
		virtual void Write(bool val)            = 0;
		virtual void Write(i8 val)              = 0;
		virtual void Write(u8 val)              = 0;
		virtual void Write(i16 val)             = 0;
		virtual void Write(u16 val)             = 0;
		virtual void Write(i32 val)             = 0;
		virtual void Write(u32 val)             = 0;
		virtual void Write(i64 val)             = 0;
		virtual void Write(u64 val)             = 0;
		virtual void Write(float val)           = 0;
		virtual void Write(double val)          = 0;
		virtual void Write(StringView val)      = 0;
		virtual bool IsValid() const            = 0;


		void PushAddFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags |= flags;
		}
		void PushRemoveFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags &= ~flags;
		}
		void PopFlags()
		{
			activeFlags = flagStack.Last();
			flagStack.RemoveAt(flagStack.Size() - 1);
		}

		sizet GetFlags() const
		{
			return activeFlags;
		}

		Writer& GetWriter()
		{
			return writer;
		}
		const Writer& GetWriter() const
		{
			return writer;
		}
		operator Writer&()
		{
			return GetWriter();
		}
	};
#pragma endregion Format Interface

#pragma region JsonFormat
	struct JsonFormatReader : public IFormatReader
	{
		enum class ReadErrorCode : u32
		{
			InvalidParameter    = 1,
			MemoryAllocation    = 2,
			EmptyContent        = 3,
			UnexpectedContent   = 4,
			UnexpectedEnd       = 5,
			UnexpectedCharacter = 6,
			JsonStructure       = 7,
			InvalidComment      = 8,
			InvalidNumber       = 9,
			InvalidString       = 10,
			ErrorLiteral        = 11,
			FileOpen            = 12,
			FileRead            = 13
		};

		struct ReadError
		{
			ReadErrorCode code;
			/** Short error message (NULL for success). */
			const char* msg;
			/** Error byte position for input data (0 for success). */
			sizet pos;
		};

	private:
		struct Scope
		{
			u32 id             = 0;
			u32 size           = 0;
			yyjson_val* parent = nullptr;
		};
		String insituBuffer;
		yyjson_doc* doc     = nullptr;
		yyjson_val* root    = nullptr;
		yyjson_val* current = nullptr;
		TArray<Scope> scopeStack;
		ReadError error;


	public:
		/**
		 * Configures a JsonFormatReader to read from an string buffer
		 * @param data containing the constant json string
		 * @see JsonFormatReader(String& data, bool insitu = true) for optional insitu reading
		 */
		PIPE_API explicit JsonFormatReader(StringView data);
		/**
		 * Configures a JsonFormatReader to read from an string buffer
		 * This contructor might MODIFY the buffer if needed to improve reading speed
		 * slightly.
		 * @param data containing the MUTABLE json string
		 */
		PIPE_API explicit JsonFormatReader(String& data);
		PIPE_API ~JsonFormatReader();

		PIPE_API void BeginObject() override;
		PIPE_API void BeginArray(u32& size) override;

		PIPE_API bool EnterNext(StringView name) override;
		PIPE_API bool EnterNext() override;
		PIPE_API void Leave() override;

		PIPE_API void Read(bool& val) override;
		PIPE_API void Read(i8& val) override;
		PIPE_API void Read(u8& val) override;
		PIPE_API void Read(i16& val) override;
		PIPE_API void Read(u16& val) override;
		PIPE_API void Read(i32& val) override;
		PIPE_API void Read(u32& val) override;
		PIPE_API void Read(i64& val) override;
		PIPE_API void Read(u64& val) override;
		PIPE_API void Read(float& val) override;
		PIPE_API void Read(double& val) override;
		PIPE_API void Read(StringView& val) override;

		PIPE_API bool IsObject() const override;
		PIPE_API bool IsArray() const override;
		PIPE_API bool IsValid() const override
		{
			return root != nullptr;
		}
		PIPE_API const ReadError& GetError() const
		{
			return error;
		}

	private:
		void InternalInit(char* data, sizet size, bool insitu);

		u32 InternalBegin();
		bool FindNextKey(u32 firstId, yyjson_val* firstKey, StringView name, u32& outIndex,
		    yyjson_val*& outValue);
		Scope& GetScope();
		void PushScope(yyjson_val* newScope);
		void PopScope();
	};


	struct JsonFormatWriter : public IFormatWriter
	{
	private:
		struct Scope
		{
			yyjson_mut_val* key    = nullptr;
			yyjson_mut_val* parent = nullptr;
		};
		yyjson_mut_doc* doc     = nullptr;
		yyjson_mut_val* current = nullptr;
		TArray<Scope> scopeStack;
		bool open = true;
		StringView asString;


	public:
		PIPE_API JsonFormatWriter();
		PIPE_API ~JsonFormatWriter();

		// BEGIN Writer Interface
		PIPE_API bool EnterNext(StringView name) override;
		PIPE_API bool EnterNext() override;
		PIPE_API void Leave() override;

		PIPE_API void BeginObject() override;
		PIPE_API void BeginArray(u32 size) override;

		PIPE_API void Write(bool val) override;
		PIPE_API void Write(i8 val) override;
		PIPE_API void Write(u8 val) override;
		PIPE_API void Write(i16 val) override;
		PIPE_API void Write(u16 val) override;
		PIPE_API void Write(i32 val) override;
		PIPE_API void Write(u32 val) override;
		PIPE_API void Write(i64 val) override;
		PIPE_API void Write(u64 val) override;
		PIPE_API void Write(float val) override;
		PIPE_API void Write(double val) override;
		PIPE_API void Write(StringView val) override;

		PIPE_API bool IsValid() const override
		{
			return doc != nullptr;
		}
		// END Writer Interface

		PIPE_API void Close();

		PIPE_API StringView ToString(bool pretty = true, bool ensureClosed = true);

	private:
		Scope& GetScope();
		void PushScope(StringView key);
		void PopScope();
	};
#pragma endregion JsonFormat

#pragma region BinaryFormat
	struct BinaryFormatReader : public IFormatReader
	{
	private:
		TView<u8> data;
		u8* pointer = nullptr;

	public:
		PIPE_API BinaryFormatReader(TView<u8> data);
		PIPE_API ~BinaryFormatReader();

		PIPE_API void BeginObject() override {}    // Nothing to do
		PIPE_API void BeginArray(u32& size) override;

		PIPE_API bool EnterNext(StringView name) override;    // Nothing to do
		PIPE_API bool EnterNext() override;                   // Nothing to do
		PIPE_API void Leave() override {}                     // Nothing to do

		PIPE_API void Read(bool& val) override;
		PIPE_API void Read(i8& val) override;
		PIPE_API void Read(u8& val) override;
		PIPE_API void Read(i16& val) override;
		PIPE_API void Read(u16& val) override;
		PIPE_API void Read(i32& val) override;
		PIPE_API void Read(u32& val) override;
		PIPE_API void Read(i64& val) override;
		PIPE_API void Read(u64& val) override;
		PIPE_API void Read(float& val) override;
		PIPE_API void Read(double& val) override;
		PIPE_API void Read(StringView& val) override;

		PIPE_API bool IsObject() const override;
		PIPE_API bool IsArray() const override;
		PIPE_API bool IsValid() const override;
	};

	struct BinaryFormatWriter : public IFormatWriter
	{
	private:
		Arena& arena;
		u8* data     = nullptr;
		i32 size     = 0;
		i32 capacity = 0;


	public:
		PIPE_API BinaryFormatWriter(Arena& arena = p::GetCurrentArena());
		PIPE_API ~BinaryFormatWriter();

		// BEGIN Writer Interface
		PIPE_API void BeginObject() override {}         // Nothing to do
		PIPE_API void BeginArray(u32 size) override;    // Nothing to do
		PIPE_API bool EnterNext(StringView name) override;
		PIPE_API bool EnterNext() override;
		PIPE_API void Leave() override {}    // Nothing to do
		PIPE_API void Write(bool val) override;
		PIPE_API void Write(i8 val) override;
		PIPE_API void Write(u8 val) override;
		PIPE_API void Write(i16 val) override;
		PIPE_API void Write(u16 val) override;
		PIPE_API void Write(i32 val) override;
		PIPE_API void Write(u32 val) override;
		PIPE_API void Write(i64 val) override;
		PIPE_API void Write(u64 val) override;
		PIPE_API void Write(float val) override;
		PIPE_API void Write(double val) override;
		PIPE_API void Write(StringView val) override;
		PIPE_API bool IsValid() const override
		{
			return data != nullptr;
		}
		// END Writer Interface

		PIPE_API TView<p::u8> GetData();

	private:
		void PreAlloc(p::u32 offset);
	};
#pragma endregion BinaryFormat
}    // namespace p
