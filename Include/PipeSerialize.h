// Copyright 2015-2026 Piperift. All Rights Reserved.
#pragma once

#include "Fwd/PipeSerialize.h"
#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Templates.h"
#include "Pipe/Core/TypeFlags.h"
#include "Pipe/Core/TypeTraits.h"
#include "PipeColor.h"
#include "PipeContainers.h"
#include "PipePlatform.h"
#include "PipeStrings.h"
#include "PipeType.h"

#include <utility>


struct yyjson_doc;
struct yyjson_mut_doc;
struct yyjson_val;
struct yyjson_mut_val;


namespace p
{
	// clang-format off
	template<typename T>
	concept Readable = requires(p::Reader reader, T v) { Read(reader, v); };
	template<typename T>
	concept Writable = requires(p::Writer writer, const T v) { Write(writer, v); };
	// clang-format on


	enum FormatFlags : sizet
	{
		FF_None                = 0,
		FF_W_CacheStringKeys   = 1 << 0,
		FF_W_CacheStringValues = 1 << 1,
		FF_W_CacheStrings      = FF_W_CacheStringKeys | FF_W_CacheStringValues,
		/** Stores member keys so the stream is self-describing:
		 * Readers can look members up by name instead of only reading them in order.
		 */
		FF_RW_Structured = 1 << 2
	};


#pragma region Reader
	struct P_API Reader : public Castable
	{
		friend IFormatReader;
		IFormatReader* formatReader = nullptr;


	private:
		// Initialize child classes using the copy constructor
		Reader() = default;

	public:
		Reader(const Reader&)            = default;
		Reader& operator=(const Reader&) = default;
		Reader(Reader&&)                 = default;
		Reader& operator=(Reader&&)      = default;
		virtual ~Reader() {}


		/**
		 * Marks current scope as an Object.
		 * See EnterNext() & Next(name, value)
		 */
		void BeginObject();

		/**
		 * Enters the next value of the current array or object.
		 * On objects, when `matchKey` is non-null, the member is entered only if its
		 * key matches; otherwise nothing is entered and false is returned. The key is
		 * ignored on arrays.
		 * @see GetCurrentKey() and GetCurrentIndex()
		 */
		bool EnterNext(StringView matchKey = {});

		/** Exits the current scope, resuming the parent one. */
		void Leave();

		/** Key of the current member if object. */
		StringView GetCurrentKey() const;
		/** Position of the current member inside its parent. */
		i32 GetCurrentIndex() const;
		/** Number of members in the current object or array scope, or NO_INDEX. */
		i32 GetCurrentSize() const;

		void PushAddFlags(FormatFlags flags);
		void PushRemoveFlags(FormatFlags flags);
		void PopFlags();
		FormatFlags GetFlags();

		/**
		 * Reads a value at a key of an object.
		 * Enters the next member only when its key matches.
		 * This function won't do anything on array or uninitialized scopes
		 */
		template<typename T>
		bool Next(StringView key, T& val)
		{
			if (EnterNext(key))
			{
				Serialize(val);
				Leave();
				return true;
			}
			return false;
		}

		/**
		 * Marks current scope as an Array.
		 * See EnterNext() & Next(value)
		 * @param size of the array being read
		 */
		void BeginArray(i32& size);

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

		inline const IFormatReader& GetFormat() const
		{
			return *formatReader;
		}

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<Reader>();
		}
	};

	// Format reads
	P_API void Read(Reader& r, bool& val);
	P_API void Read(Reader& r, i8& val);
	P_API void Read(Reader& r, u8& val);
	P_API void Read(Reader& r, i16& val);
	P_API void Read(Reader& r, u16& val);
	P_API void Read(Reader& r, i32& val);
	P_API void Read(Reader& r, u32& val);
	P_API void Read(Reader& r, i64& val);
	P_API void Read(Reader& r, u64& val);
	P_API void Read(Reader& r, float& val);
	P_API void Read(Reader& r, double& val);
	P_API void Read(Reader& r, StringView& val);

	template<typename T1, typename T2>
	void Read(Reader& r, p::TPair<T1, T2>& val)
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
		i32 size;
		r.BeginArray(size);
		val.Resize(size);
		for (i32 i = 0; i < size; ++i)
		{
			r.Next(val[i]);
		}
	}
#pragma endregion Reader


#pragma region Writer
	struct P_API Writer : public Castable
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
		 * @see EnterNext(matchKey) & Next(name, value)
		 */
		void BeginObject();

		/**
		 * Enters and adds next array index or key value of an object.
		 * Complexity: O(1)
		 * To make object scope, see 'BeginObject()'
		 * @return true if the value could be added to the current scope.
		 */
		bool EnterNext(StringView key = {});

		/**
		 * Writes a type at a key of an object.
		 * Complexity: O(1)
		 * This function won't do anything on array or uninitialized scopes
		 */
		template<typename T>
		void Next(StringView key, const T& val)
		{
			if (EnterNext(key))
			{
				Serialize(val);
				Leave();
			}
		}

		/**
		 * Marks current scope as an Array.
		 * See EnterNext() & Next(value)
		 * @param size of the array being written
		 */
		void BeginArray(i32 size);

		/**
		 * Writes a type into the next element of an array.
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


		/** Writes a type into the current value. */
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

		/** Exits the current scope, resuming the parent one. */
		void Leave();

		static constexpr bool IsReading()
		{
			return false;
		}
		static constexpr bool IsWriting()
		{
			return true;
		}

		void PushAddFlags(FormatFlags flags);
		void PushRemoveFlags(FormatFlags flags);
		void PopFlags();
		FormatFlags GetFlags();

		inline IFormatWriter& GetFormat()
		{
			return *formatWriter;
		}

		inline const IFormatWriter& GetFormat() const
		{
			return *formatWriter;
		}

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<Writer>();
		}
	};

	// Format writes
	P_API void Write(Writer& w, bool val);
	P_API void Write(Writer& w, i8 val);
	P_API void Write(Writer& w, u8 val);
	P_API void Write(Writer& w, i16 val);
	P_API void Write(Writer& w, u16 val);
	P_API void Write(Writer& w, i32 val);
	P_API void Write(Writer& w, u32 val);
	P_API void Write(Writer& w, i64 val);
	P_API void Write(Writer& w, u64 val);
	P_API void Write(Writer& w, float val);
	P_API void Write(Writer& w, double val);
	P_API void Write(Writer& w, StringView val);

	template<typename T1, typename T2>
	void Write(Writer& w, const p::TPair<T1, T2>& val)
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
	void Write(Writer& w, const T& val) requires(IsArray<T>())
	{
		const i32 size = i32(val.Size());
		w.BeginArray(size);
		for (i32 i = 0; i < size; ++i)
		{
			w.Next(val[i]);
		}
	}
#pragma endregion Writer


#pragma region ReadWriter
	struct P_API ReadWriter
	{
	private:
		Reader* reader;
		Writer* writer;


	public:
		ReadWriter(Reader& reader) : reader{&reader}, writer{nullptr} {}
		ReadWriter(Writer& writer) : reader{nullptr}, writer{&writer} {}
		ReadWriter(const ReadWriter& other)            = default;
		ReadWriter& operator=(const ReadWriter& other) = default;
		ReadWriter(ReadWriter&& other)                 = default;
		ReadWriter& operator=(ReadWriter&& other)      = default;

		/**
		 * Marks current value as an Object.
		 * See EnterNext(key) & Next(key, value)
		 */
		void BeginObject();

		/** Enters the next object member or array element. Readers match `matchKey` when
		 * non-null; writers write it.
		 * @see GetCurrentKey() and GetCurrentIndex()
		 */
		bool EnterNext(StringView key = {});

		/** Reads or writes a value at a key of an object. */
		template<typename T>
		void Next(StringView key, T& val)
		{
			if (IsWriting())
			{
				writer->Next(key, val);
			}
			else
			{
				reader->Next(key, val);
			}
		}


		/**
		 * Marks current value as an Array.
		 * See EnterNext() & Next(value)
		 * @param size of the array being read or written
		 */
		void BeginArray(i32& size);

		/**
		 * Reads or writes the next element of an array.
		 * Complexity: O(1)
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

		/** Reads or writes a type from the current scope. */
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

		/** Exits the current scope, resuming the parent one. */
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

		void PushAddFlags(FormatFlags flags);
		void PushRemoveFlags(FormatFlags flags);
		void PopFlags();
	};


	// Types can be marked as single serialize, so that Serialize() will be called instead of Read
	// and Write
	template<typename T>
	void Read(Reader& ct, T& val)
	    requires(bool(TFlags<T>::HasSingleSerialize && TFlags<T>::HasMemberSerialize))
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
	    requires(bool(TFlags<T>::HasSingleSerialize && TFlags<T>::HasMemberSerialize))
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
	struct P_API IFormatReader
	{
	private:
		Reader reader{};

		sizet activeFlags = FF_None;
		TArray<sizet> flagStack;


	public:
		IFormatReader()
		{
			reader.formatReader = this;
		}

		IFormatReader(const IFormatReader&)            = delete;
		IFormatReader& operator=(const IFormatReader&) = delete;
		IFormatReader(IFormatReader&& other) noexcept
		    : reader{Move(other.reader)}
		    , activeFlags{other.activeFlags}
		    , flagStack{Move(other.flagStack)}
		{
			reader.formatReader = this;
		}
		IFormatReader& operator=(IFormatReader&& other) noexcept
		{
			if (this != &other)
			{
				activeFlags         = other.activeFlags;
				flagStack           = Move(other.flagStack);
				reader.formatReader = this;
			}
			return *this;
		}

		virtual ~IFormatReader() = default;
		/** Marks the current value as an object scope. */
		virtual void BeginObject() = 0;
		/** Marks the current value as an array scope. */
		virtual void BeginArray(i32& size) = 0;
		/** Enters the next value of the current array or object.
		 * On objects, when `matchKey` is non-null, the member is entered only if its
		 * key matches; otherwise nothing is entered and false is returned. The key is
		 * ignored on arrays.
		 * @see GetCurrentKey() and GetCurrentIndex()
		 */
		virtual bool EnterNext(StringView matchKey = {}) = 0;
		/** Exits the current scope. */
		virtual void Leave() = 0;
		/** Key of the last entered member; empty when it was unnamed. */
		virtual StringView GetCurrentKey() const = 0;
		/** Position of the last entered member inside its scope. */
		virtual i32 GetCurrentIndex() const = 0;
		/** Number of members in the current object or array scope, or NO_INDEX. */
		virtual i32 GetCurrentSize() const = 0;
		virtual void Read(bool& val)        = 0;
		virtual void Read(i8& val)          = 0;
		virtual void Read(u8& val)          = 0;
		virtual void Read(i16& val)         = 0;
		virtual void Read(u16& val)         = 0;
		virtual void Read(i32& val)         = 0;
		virtual void Read(u32& val)         = 0;
		virtual void Read(i64& val)         = 0;
		virtual void Read(u64& val)         = 0;
		virtual void Read(float& val)       = 0;
		virtual void Read(double& val)      = 0;
		virtual void Read(StringView& val)  = 0;
		virtual bool IsObject() const       = 0;
		virtual bool IsArray() const        = 0;
		virtual bool IsValid() const        = 0;

		void PushAddFlags(FormatFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags |= flags;
		}
		void PushRemoveFlags(FormatFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags &= ~sizet(flags);
		}
		void PopFlags()
		{
			activeFlags = flagStack.Last();
			flagStack.RemoveAt(flagStack.Size() - 1);
		}
		FormatFlags GetFlags() const
		{
			return FormatFlags(activeFlags);
		}

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


	struct P_API IFormatWriter
	{
	private:
		Writer writer{};


		sizet activeFlags = FF_None;
		TArray<sizet> flagStack;

	public:
		IFormatWriter()
		{
			writer.formatWriter = this;
		}

		IFormatWriter(const IFormatWriter&)            = delete;
		IFormatWriter& operator=(const IFormatWriter&) = delete;
		IFormatWriter(IFormatWriter&& other) noexcept
		    : writer{Move(other.writer)}
		    , activeFlags{other.activeFlags}
		    , flagStack{Move(other.flagStack)}
		{
			writer.formatWriter = this;
		}
		IFormatWriter& operator=(IFormatWriter&& other) noexcept
		{
			if (this != &other)
			{
				activeFlags         = other.activeFlags;
				flagStack           = Move(other.flagStack);
				writer.formatWriter = this;
			}
			return *this;
		}

		virtual ~IFormatWriter() = default;
		/** Marks the current value as an object scope. */
		virtual void BeginObject() = 0;
		/** Marks the current value as an array scope. */
		virtual void BeginArray(i32 size)           = 0;
		virtual bool EnterNext(StringView key = {}) = 0;
		/** Exits the current scope. */
		virtual void Leave()               = 0;
		virtual void Write(bool val)       = 0;
		virtual void Write(i8 val)         = 0;
		virtual void Write(u8 val)         = 0;
		virtual void Write(i16 val)        = 0;
		virtual void Write(u16 val)        = 0;
		virtual void Write(i32 val)        = 0;
		virtual void Write(u32 val)        = 0;
		virtual void Write(i64 val)        = 0;
		virtual void Write(u64 val)        = 0;
		virtual void Write(float val)      = 0;
		virtual void Write(double val)     = 0;
		virtual void Write(StringView val) = 0;
		virtual bool IsValid() const       = 0;


		void PushAddFlags(FormatFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags |= flags;
		}
		void PushRemoveFlags(FormatFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags &= ~flags;
		}
		void PopFlags()
		{
			activeFlags = flagStack.Last();
			flagStack.RemoveAt(flagStack.Size() - 1);
		}

		FormatFlags GetFlags() const
		{
			return FormatFlags(activeFlags);
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
		struct Value
		{
			yyjson_val* value = nullptr;     // The value itself
			i32 index         = NO_INDEX;    // This value's index inside its parent (arrays)
			StringView key;                  // This value's key (objects)
			yyjson_val* last = nullptr;      // Last member entered/left from this value
			i32 lastIndex    = NO_INDEX;     // Index of that member
		};
		yyjson_doc* doc  = nullptr;
		yyjson_val* root = nullptr;
		Value current{};             // Value being read
		TArray<Value> valueStack;    // Values enclosing `current`
		ReadError error;


	public:
		/**
		 * Configures a JsonFormatReader to read from an string buffer
		 * @param data containing the constant json string
		 * @see JsonFormatReader(String& data, bool insitu = true) for optional insitu reading
		 */
		JsonFormatReader(const JsonFormatReader&)            = delete;
		JsonFormatReader& operator=(const JsonFormatReader&) = delete;
		P_API JsonFormatReader(JsonFormatReader&& other) noexcept;
		P_API JsonFormatReader& operator=(JsonFormatReader&& other) noexcept;
		P_API explicit JsonFormatReader(StringView data);
		/**
		 * Configures a JsonFormatReader to read from an string buffer
		 * This contructor might MODIFY the buffer if needed to improve reading speed
		 * slightly.
		 * @param data containing the MUTABLE json string
		 */
		P_API explicit JsonFormatReader(String& data);
		P_API ~JsonFormatReader();

		P_API void BeginObject() override;
		P_API void BeginArray(i32& size) override;

		P_API bool EnterNext(StringView matchKey = {}) override;
		P_API void Leave() override;
		P_API StringView GetCurrentKey() const override;
		P_API i32 GetCurrentIndex() const override;
		P_API i32 GetCurrentSize() const override;

		P_API void Read(bool& val) override;
		P_API void Read(i8& val) override;
		P_API void Read(u8& val) override;
		P_API void Read(i16& val) override;
		P_API void Read(u16& val) override;
		P_API void Read(i32& val) override;
		P_API void Read(u32& val) override;
		P_API void Read(i64& val) override;
		P_API void Read(u64& val) override;
		P_API void Read(float& val) override;
		P_API void Read(double& val) override;
		P_API void Read(StringView& val) override;

		P_API bool IsObject() const override;
		P_API bool IsArray() const override;
		P_API bool IsValid() const override
		{
			return root != nullptr;
		}
		P_API const ReadError& GetError() const
		{
			return error;
		}

	private:
		void InternalInit(char* data, sizet size, bool insitu);

		void PushValue(yyjson_val* newValue, i32 newIndex = NO_INDEX, StringView newKey = {});
		void PopValue();
	};


	struct JsonFormatWriter : public IFormatWriter
	{
	private:
		struct Value
		{
			yyjson_mut_val* parent = nullptr;
			yyjson_mut_val* key    = nullptr;
			i32 index              = NO_INDEX;    // Array element to replace, NO_INDEX otherwise
		};
		yyjson_mut_doc* doc     = nullptr;
		yyjson_mut_val* current = nullptr;
		TArray<Value> valueStack;
		bool open = true;
		StringView asString;


	public:
		JsonFormatWriter(const JsonFormatWriter&)            = delete;
		JsonFormatWriter& operator=(const JsonFormatWriter&) = delete;
		P_API JsonFormatWriter(JsonFormatWriter&& other) noexcept;
		P_API JsonFormatWriter& operator=(JsonFormatWriter&& other) noexcept;
		P_API JsonFormatWriter();
		P_API ~JsonFormatWriter();

		// BEGIN Writer Interface
		P_API bool EnterNext(StringView key = {}) override;
		P_API void Leave() override;

		P_API void BeginObject() override;
		P_API void BeginArray(i32 size) override;

		P_API void Write(bool val) override;
		P_API void Write(i8 val) override;
		P_API void Write(u8 val) override;
		P_API void Write(i16 val) override;
		P_API void Write(u16 val) override;
		P_API void Write(i32 val) override;
		P_API void Write(u32 val) override;
		P_API void Write(i64 val) override;
		P_API void Write(u64 val) override;
		P_API void Write(float val) override;
		P_API void Write(double val) override;
		P_API void Write(StringView val) override;

		P_API bool IsValid() const override
		{
			return doc != nullptr;
		}
		// END Writer Interface

		P_API void Close();

		P_API StringView ToString(bool pretty = true, bool ensureClosed = true);

	private:
		Value& GetCurrentValue();
		void PushValue(StringView key);
		void PushValue(i32 index);
		void PopValue();
	};
#pragma endregion JsonFormat

#pragma region BinaryFormat
	namespace Binary
	{
		enum class Flags : u8;
	}

	struct BinaryFormatReader : public IFormatReader
	{
	private:
		/** One value: bounds, identity, iteration and framing. */
		struct Value
		{
			u32 offset          = 0;           // First byte of the value, from the stream start
			Binary::Flags flags = {};          // Value type
			i32 index           = NO_INDEX;    // This value's index inside its parent (arrays)
			StringView key;                    // This value's key (objects)
			i32 lastIndex = NO_INDEX;          // Last element entered/left from this value
			i32 size      = NO_INDEX;          // Array element count
			bool structuredChildren = false;
		};

		TView<u8> data;
		u8* pointer = nullptr;

		Value current;
		TArray<Value> valueStack;

		sizet Remaining() const;
		u8* ReadBytes(sizet size);

		/** Enters a value, moving the enclosing one to the stack. */
		void PushValue(Value value);
		/** Leaves the current value, skipping whatever the caller did not read. */
		void PopValue();

	public:
		BinaryFormatReader(const BinaryFormatReader&)                            = delete;
		BinaryFormatReader& operator=(const BinaryFormatReader&)                 = delete;
		P_API BinaryFormatReader(BinaryFormatReader&& other) noexcept            = default;
		P_API BinaryFormatReader& operator=(BinaryFormatReader&& other) noexcept = default;
		P_API BinaryFormatReader(TView<u8> data);
		P_API ~BinaryFormatReader();

		P_API void BeginObject() override;
		P_API void BeginArray(i32& size) override;

		P_API bool EnterNext(StringView matchKey = {}) override;
		P_API void Leave() override;
		P_API StringView GetCurrentKey() const override;
		P_API i32 GetCurrentIndex() const override;
		P_API i32 GetCurrentSize() const override;

		P_API void Read(bool& val) override;
		P_API void Read(i8& val) override;
		P_API void Read(u8& val) override;
		P_API void Read(i16& val) override;
		P_API void Read(u16& val) override;
		P_API void Read(i32& val) override;
		P_API void Read(u32& val) override;
		P_API void Read(i64& val) override;
		P_API void Read(u64& val) override;
		P_API void Read(float& val) override;
		P_API void Read(double& val) override;
		P_API void Read(StringView& val) override;

		P_API bool IsObject() const override;
		P_API bool IsArray() const override;
		P_API bool IsValid() const override;

		/** True once a read tried to exceed the buffer. */
		P_API bool HasError() const;
	};

	struct BinaryFormatWriter : public IFormatWriter
	{
	private:
		/** One value: payload position, scope flags and scope framing. */
		struct Value
		{
			u32 offset              = 0;     // First byte of the payload
			Binary::Flags flags     = {};    // Value type
			bool structuredChildren = false;
			i32 size                = 0;     // Elements entered in this object/array
		};

		Arena* arena = nullptr;
		u8* data     = nullptr;
		u32 size     = 0;
		u32 capacity = 0;

		Value current;
		TArray<Value> valueStack;


	public:
		BinaryFormatWriter(const BinaryFormatWriter&)            = delete;
		BinaryFormatWriter& operator=(const BinaryFormatWriter&) = delete;
		P_API BinaryFormatWriter(BinaryFormatWriter&& other) noexcept;
		P_API BinaryFormatWriter& operator=(BinaryFormatWriter&& other) noexcept;
		P_API BinaryFormatWriter(Arena& arena = p::GetCurrentArena());
		P_API ~BinaryFormatWriter();

		// BEGIN Writer Interface
		P_API void BeginObject() override;
		P_API void BeginArray(i32 size) override;
		P_API bool EnterNext(StringView key = {}) override;
		P_API void Leave() override;
		P_API void Write(bool val) override;
		P_API void Write(i8 val) override;
		P_API void Write(u8 val) override;
		P_API void Write(i16 val) override;
		P_API void Write(u16 val) override;
		P_API void Write(i32 val) override;
		P_API void Write(u32 val) override;
		P_API void Write(i64 val) override;
		P_API void Write(u64 val) override;
		P_API void Write(float val) override;
		P_API void Write(double val) override;
		P_API void Write(StringView val) override;
		P_API bool IsValid() const override
		{
			return data != nullptr;
		}
		// END Writer Interface

		P_API TView<p::u8> GetData();

	private:
		void PreAlloc(p::u32 offset);
	};
#pragma endregion BinaryFormat


#pragma region CoreSupport
	struct Tag;
	struct Guid;
	template<u32 size, Number T>
	struct Vec;
	struct Quat;

	P_API void Read(Reader& ct, String& val);
	P_API void Write(Writer& ct, const String& val);
	P_API void Read(Reader& ct, Tag& val);
	P_API void Write(Writer& ct, const Tag& val);

	P_API void Read(Reader& ct, Guid& guid);
	P_API void Write(Writer& ct, const Guid& guid);

	P_API void Read(Reader& ct, TColor<ColorMode::RGBA>& color);
	P_API void Write(Writer& ct, const TColor<ColorMode::RGBA>& color);
	P_API void Read(Reader& ct, TColor<ColorMode::Linear>& color);
	P_API void Write(Writer& ct, const TColor<ColorMode::Linear>& color);
	P_API void Read(Reader& ct, TColor<ColorMode::sRGB>& color);
	P_API void Write(Writer& ct, const TColor<ColorMode::sRGB>& color);
	P_API void Read(Reader& r, TColor<ColorMode::HSV>& color);
	P_API void Write(Writer& w, const TColor<ColorMode::HSV>& color);

	P_API void Read(Reader& ct, Vec<2, float>& val);
	P_API void Write(Writer& ct, const Vec<2, float>& val);
	P_API void Read(Reader& ct, Vec<2, u32>& val);
	P_API void Write(Writer& ct, const Vec<2, u32>& val);
	P_API void Read(Reader& ct, Vec<2, i32>& val);
	P_API void Write(Writer& ct, const Vec<2, i32>& val);
	P_API void Read(Reader& ct, Vec<3, float>& val);
	P_API void Write(Writer& ct, const Vec<3, float>& val);
	P_API void Read(Reader& ct, Vec<3, u32>& val);
	P_API void Write(Writer& ct, const Vec<3, u32>& val);
	P_API void Read(Reader& ct, Vec<3, i32>& val);
	P_API void Write(Writer& ct, const Vec<3, i32>& val);
	P_API void Read(Reader& ct, Quat& val);
	P_API void Write(Writer& ct, const Quat& val);
#pragma endregion CoreSupport
}    // namespace p
