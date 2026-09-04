# Log

The logging system in Pipe provides a callback-based logger with three severity levels: `Info`, `Warning`, and `Error`.

## Default Behavior

The default logger outputs messages with no formatting:

```cpp
p::Info("hello world");   // prints: hello world
p::Warning("something");  // prints: something
p::Error("bad state");    // prints: bad state
```

## API

### Logger Struct

```cpp
struct Logger
{
    std::function<void(StringView)> infoCallback;
    std::function<void(StringView)> warningCallback;
    std::function<void(StringView)> errorCallback;
};
```

Each field is a callback invoked for its respective log level. All three must be provided.

### InitLog / ShutdownLog

```cpp
void InitLog(Logger* logger = nullptr);
void ShutdownLog();
```

- `InitLog(nullptr)` — uses the default logger.
- `InitLog(&myLogger)` — uses a custom logger. The pointer must remain valid until `ShutdownLog()`.
- `ShutdownLog()` — clears the active logger. Calls to `Info`/`Warning`/`Error` become no-ops until `InitLog` is called again.

### Info / Warning / Error

```cpp
void Info(StringView msg);
void Warning(StringView msg);
void Error(StringView msg);
```

These dispatch to the corresponding callback on the active logger. If no logger is active messages are silently ignored.

Text can be formatted (using `p::Format`):

```cpp
p::Info("Loaded {} assets.", count);
p::Warning("Deprecated: {}", feature);
p::Error("Failed to open {}", path);
```

## Custom Logger Example

The following reproduces the old default format with timestamps and level tags:

```cpp
p::Logger timestampedLogger{
    .infoCallback = [](p::StringView msg) {
        p::String text;
        auto now = p::DateTime::Now();
        now.ToString("[%Y/%m/%d %H:%M:%S]", text);
        p::FormatTo(text, "[Info] {}\n", msg);
        std::cout << text;
    },
    .warningCallback = [](p::StringView msg) {
        p::String text;
        auto now = p::DateTime::Now();
        now.ToString("[%Y/%m/%d %H:%M:%S]", text);
        p::FormatTo(text, "[Warning] {}\n", msg);
        std::cout << text;
    },
    .errorCallback = [](p::StringView msg) {
        p::String text;
        auto now = p::DateTime::Now();
        now.ToString("[%Y/%m/%d %H:%M:%S]", text);
        p::FormatTo(text, "[Error] {}\n", msg);
        std::cout << text;
    }
};

p::InitLog(&timestampedLogger);
p::Info("Server Started"); // [2026/09/04 12:00:00] [Info] server started
```

Outputs:
```
[2026/09/04 12:00:00] [Info] server started
```
