#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>
#include <algorithm>

// TODO closures (Prototype + Environment)
// TODO default functions (car, cdr, cons, +, -, *, /, print, def)
// TODO evaluation
// TODO parsing
// TODO active garbage collection (mark and sweep)

#ifndef TINY_LISP_HPP
#define TINY_LISP_HPP

enum class LispValueType: uint8_t {
    Nil,
    Symbol,
    Number,
    Cons, 
    Closure,
    Error,
};

inline std::string Enum2String(LispValueType type) {
    switch (type) {
        case LispValueType::Nil:
            return "nil";
        case LispValueType::Symbol:
            return "symbol";
        case LispValueType::Number:
            return "number";
        case LispValueType::Cons:
            return "cons";
        case LispValueType::Closure:
            return "closure";
        case LispValueType::Error:
            return "error";
        default:
            return "<unknown-type>";
    }
}

enum class LispErrorCode: uint8_t {
    None,
    Generic,

    ParsingError,

    ArgumentCountMismatch,

    MemoryOutOfSpace,
    MemoryUninitialized,
    MemoryAccessOutOfBounds,

    ArithmeticNonNumber,
    ArithmeticDivideByZero,
};

inline std::string Enum2String(LispErrorCode type) {
    switch (type) {
        case LispErrorCode::None:
            return "no error";
        case LispErrorCode::Generic:
            return "generic error";
        case LispErrorCode::ParsingError:
            return "parsing error";
        case LispErrorCode::ArgumentCountMismatch:
            return "argument count mismatch";
        case LispErrorCode::MemoryOutOfSpace:
            return "out of memory";
        case LispErrorCode::MemoryUninitialized:
            return "using uninitilized memory";
        case LispErrorCode::MemoryAccessOutOfBounds:
            return "index out of bounds";
        case LispErrorCode::ArithmeticNonNumber:
            return "not a number";
        case LispErrorCode::ArithmeticDivideByZero:
            return "divide by zero";
        default:
            return "<unknown-type>";
    }
}

enum class ParseErrorCode: uint8_t
{
    None,
    Generic,
    MissingClosingParen,
    InvalidToken,
    EndOfFile,
};

inline std::string Enum2String(ParseErrorCode type) {
    switch (type) {
        case ParseErrorCode::None:
            return "no error";
        case ParseErrorCode::Generic:
            return "generic error";
        case ParseErrorCode::MissingClosingParen:
            return "missing closing parenthesis";
        case ParseErrorCode::InvalidToken:
            return "invalid token";
        case ParseErrorCode::EndOfFile:
            return "unexpected end of file";
        default:
            return "<unknown-type>";
    }
}


typedef int LispNumber;

typedef unsigned int SymbolRef;

typedef uint16_t LispRef;

class LispValue {
public:
    union {
        LispNumber Number;
        SymbolRef Symbol;
        struct {
            LispRef Car;
            LispRef Cdr;
        } Cons;
        struct {
            LispRef Body; 
            LispRef Environment;
        } Closure;
    } As;
    LispValueType Type;
    LispErrorCode ErrorType;

private:
    LispValue() : Type(LispValueType::Nil), ErrorType(LispErrorCode::None) {}

    template <typename Func>
    LispValue Binop(const LispValue &other, Func f) const {
        if (IsError()) return *this;
        if (other.IsError()) return other;
        return f(*this, other);
    }

public:

    static LispValue Nil() {
        LispValue v;
        v.Type = LispValueType::Nil;
        v.ErrorType = LispErrorCode::None;
        v.As.Number = 0;
        return v;
    }

    static LispValue Number(LispNumber number) {
        LispValue v;
        v.Type = LispValueType::Number;
        v.ErrorType = LispErrorCode::None;
        v.As.Number = number;
        return v;
    }

    static LispValue Symbol(SymbolRef reference) {
        LispValue v;
        v.Type = LispValueType::Symbol;
        v.ErrorType = LispErrorCode::None;
        v.As.Symbol = reference;
        return v;
    }

    static LispValue Cons(LispRef car, LispRef cdr) {
        LispValue v;
        v.Type = LispValueType::Cons;
        v.ErrorType = LispErrorCode::None;
        v.As.Cons.Car = car;
        v.As.Cons.Cdr = cdr;
        return v;
    }

    static LispValue Closure(LispRef body, LispRef environment) {
        LispValue v;
        v.Type = LispValueType::Closure;
        v.ErrorType = LispErrorCode::None;
        v.As.Closure.Body = body;
        v.As.Closure.Environment = environment;
        return v;
    }

    static LispValue Error(LispErrorCode err) {
        LispValue v;
        v.Type = LispValueType::Error;
        v.ErrorType = err;
        v.As.Number = 0;
        return v;
    }

    bool Eq(const LispValue &other) const {
        if (this->Type != other.Type)
            return false;

        switch (this->Type) {
            case LispValueType::Nil:
                return true;
            case LispValueType::Error:
                return this->ErrorType == other.ErrorType;
            case LispValueType::Number:
                return this->As.Number == other.As.Number;
            case LispValueType::Symbol:
                return this->As.Symbol == other.As.Symbol;
            case LispValueType::Cons:
                return this->As.Cons.Car == other.As.Cons.Car && this->As.Cons.Cdr == other.As.Cons.Cdr;
            case LispValueType::Closure:
                return this->As.Closure.Body == other.As.Closure.Body && this->As.Closure.Environment == other.As.Closure.Environment;
            default:
                return false;
        }
    }

    bool AsBoolean() const {
        switch (this->Type) {
            case LispValueType::Nil:
            case LispValueType::Error:
                return false;

            case LispValueType::Number:
                return this->As.Number != 0;

            default:
                return true;
        }
    }

    bool IsAtom() const {
        switch (this->Type) {
            case LispValueType::Nil:
            case LispValueType::Number:
            case LispValueType::Symbol:
            case LispValueType::Error:
                return true;

            default:
                return false;
        }
    }

    bool IsList() const {
        switch (this->Type) {
            case LispValueType::Nil:
            case LispValueType::Cons:
            case LispValueType::Closure:
                return true;
            
            default:
                return false;
        }
    }

    bool IsNil() const {
        return this->Type == LispValueType::Nil;
    }

    bool IsError() const {
        return this->Type == LispValueType::Error;
    }

    bool IsNumber() const {
        return this->Type == LispValueType::Number;
    }

    bool IsSymbol() const {
        return this->Type == LispValueType::Symbol;
    }

    bool IsCons() const {
        return this->Type == LispValueType::Cons;
    }

    bool IsClosure() const {
        return this->Type == LispValueType::Closure;
    }

    bool Gt(const LispValue &other) const {
        if (!this->IsNumber())  
            return false;
        if (!other.IsNumber())
            return false;
        return this->As.Number > other.As.Number;
    }

    bool Lt(const LispValue &other) const {
        if (!this->IsNumber())  
            return false;
        if (!other.IsNumber())
            return false;
        return this->As.Number < other.As.Number;
    }

    LispValue Add(const LispValue &other) const {
        return Binop(other, [](const LispValue &lhs, const LispValue &rhs) { return lhs.IsNumber() && rhs.IsNumber() ? Number(lhs.As.Number + rhs.As.Number) : Error(LispErrorCode::ArithmeticNonNumber); });
    }

    LispValue Sub(const LispValue &other) const {
        return Binop(other, [](const LispValue &lhs, const LispValue &rhs) { return lhs.IsNumber() && rhs.IsNumber() ? Number(lhs.As.Number - rhs.As.Number) : Error(LispErrorCode::ArithmeticNonNumber); });
    }

    LispValue Mul(const LispValue &other) const {
        return Binop(other, [](const LispValue &lhs, const LispValue &rhs) { return lhs.IsNumber() && rhs.IsNumber() ? Number(lhs.As.Number * rhs.As.Number) : Error(LispErrorCode::ArithmeticNonNumber); });
    }

    LispValue Div(const LispValue &other) const {
        return Binop(other, [](const LispValue &lhs, const LispValue &rhs) { return lhs.IsNumber() && rhs.IsNumber() ? (rhs.As.Number != 0 ? Number(lhs.As.Number / rhs.As.Number) : Error(LispErrorCode::ArithmeticDivideByZero)) : Error(LispErrorCode::ArithmeticNonNumber); });
    }
};

enum class MemoryFlags: uint8_t {
    None = 0,           // No flags placeholder
    Used = 1 << 0,      // Used indicates that it contains a real value
    Marked = 1 << 1,    // Marked indicates it can be freed at a later time
    Protected = 1 << 2,  // Protected indicates it can't be freed
};

inline MemoryFlags operator|(MemoryFlags a, MemoryFlags b) {
    return static_cast<MemoryFlags>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
    );
}

inline MemoryFlags operator&(MemoryFlags a, MemoryFlags b) {
    return static_cast<MemoryFlags>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
    );
}

inline MemoryFlags operator~(MemoryFlags a) {
    return static_cast<MemoryFlags>(
        ~static_cast<uint8_t>(a)
    );
}

inline bool operator == (MemoryFlags a, int b) {
    return static_cast<int>(a) == b;
}

inline bool operator != (MemoryFlags a, int b) {
    return static_cast<int>(a) != b;
}

inline void operator |= (MemoryFlags &a, MemoryFlags b) {
    a = static_cast<MemoryFlags>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b)
    );
}

inline void operator &= (MemoryFlags &a, MemoryFlags b) {
    a = static_cast<MemoryFlags>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b)
    );
}

class MemoryCell {
public:
    MemoryFlags Flags;
    LispValue Value;
    MemoryCell(): Flags(MemoryFlags::None), Value(LispValue::Error(LispErrorCode::MemoryUninitialized)) {}
};

enum class BuiltinFunction {
    Define,
    Lambda,
    Car,
    Cdr,
    Cons,
    Add,
    Sub,
    Mul,
    Div,
    IsAtom,
    IsList,
    Quote,
    Greater,
    Less,
    Equals,
    Conds,
    And,
    Or,
    Xor,
    Not
};

struct SymbolTableEntryProperties {
    LispRef BoundReference;
    bool ReBindable;
    bool Protected;

    SymbolTableEntryProperties(): BoundReference(0), ReBindable(true), Protected(false) {}
};

struct SymbolTableEntry {
    std::string Text;
    SymbolTableEntryProperties Properties;

    SymbolTableEntry(): Text(), Properties() {}
};

class SymbolTable {
private:
    std::string empty;
    std::vector<SymbolTableEntry> Symbols;

    using iterator = std::vector<SymbolTableEntry>::iterator;
    using const_iterator = std::vector<SymbolTableEntry>::const_iterator;

public:
    SymbolTable() : empty(""), Symbols() {
        // Special symbols (see BuiltinFunction enum) or TryExecBuiltin for implementation
        SymbolTableEntryProperties defaultProperties;
        defaultProperties.BoundReference = static_cast<LispRef>(0);
        defaultProperties.Protected = true;
        defaultProperties.ReBindable = false;

        Intern("define",    defaultProperties);
        Intern("lambda",    defaultProperties);   
        Intern("car",       defaultProperties);
        Intern("cdr",       defaultProperties);
        Intern("cons",      defaultProperties);
        Intern("+",         defaultProperties);
        Intern("-",         defaultProperties);
        Intern("*",         defaultProperties);
        Intern("/",         defaultProperties);
        Intern("atom?",     defaultProperties);
        Intern("list?",     defaultProperties);
        Intern("quote",     defaultProperties);
        Intern(">",         defaultProperties);
        Intern("<",         defaultProperties);
        Intern("=",         defaultProperties);
        Intern("cond",      defaultProperties);
        Intern("and",       defaultProperties);
        Intern("or",        defaultProperties);
        Intern("xor",       defaultProperties);
        Intern("not",       defaultProperties);
    }

    iterator begin() { return Symbols.begin(); }
    iterator end() { return Symbols.end(); }
    const_iterator begin() const { return Symbols.begin(); }
    const_iterator end() const { return Symbols.end(); }

    /// @brief intern a symbol text with default properties
    /// @param name symbol name
    /// @return symbol reference
    SymbolRef Intern(const std::string &name) {
        // Check if symbol already exists (linear search)
        for (std::size_t i = 0; i < Symbols.size(); i++) {
            if (Symbols[i].Text == name) {
                return static_cast<SymbolRef>(i);
            }
        }
        
        // Create new symbol
        SymbolRef ref = static_cast<SymbolRef>(Symbols.size());
        SymbolTableEntry entry;
        entry.Text = name;
        Symbols.push_back(entry);
        return ref;
    }

    /// @brief intern a symbol text and if its a new symbol assign the given properties
    /// @param name symbol name
    /// @param properties symbol properties
    /// @return symbol reference
    SymbolRef Intern(const std::string &name, const SymbolTableEntryProperties &properties) {
        // Check if symbol already exists (linear search)
        for (std::size_t i = 0; i < Symbols.size(); i++) {
            if (Symbols[i].Text == name) {
                return static_cast<SymbolRef>(i);
            }
        }
        
        // Create new symbol (and make its properties the desired properties)
        SymbolRef ref = static_cast<SymbolRef>(Symbols.size());
        SymbolTableEntry entry;
        entry.Text = name;
        entry.Properties = properties;
        Symbols.push_back(entry);
        return ref;
    }

    /// @brief bind a given symbol reference to a value on the runtime's heap
    bool Bind(SymbolRef symbol, LispRef value) {
        auto index = static_cast<std::vector<SymbolTableEntry>::size_type>(symbol);
        if (index >= Symbols.size())
            return false; // Do nothing if out of range

        SymbolTableEntry &entry = Symbols[index];
        if (!entry.Properties.ReBindable)
            return false; // Do nothing if not bindable

        entry.Properties.BoundReference = value;
        return true;
    }

    /// @brief clear all symbols from the symbol table except protected symbols (usually builtins)
    void Clear() {
        // Clear all except protected symbols
        Symbols.erase(
            std::remove_if(Symbols.begin(), Symbols.end(), [](const SymbolTableEntry& sym) {
                return !sym.Properties.Protected;
            }),
            Symbols.end()
        );
        Symbols.shrink_to_fit();
    }
    
    /// @brief get the name associated with the given symbol reference
    /// @param ref reference to the symbol
    /// @return string
    const std::string& GetName(SymbolRef ref) const {
        if (ref >= Symbols.size()) return empty;
        return Symbols[ref].Text;
    }
};

class ICharStream {
public:
    virtual bool Eof() const = 0;
    virtual char Peek() const = 0;
    virtual char PeekFuture() const = 0;
    virtual char Advance() = 0;
};

class LispRuntime {
private:
    SymbolTable Symbols;
    std::vector<MemoryCell> Heap;
    std::vector<std::vector<MemoryCell>::size_type> HeapFreeList;
    std::vector<LispRef> rootset;
    std::vector<MemoryCell>::size_type allocatedSinceLastGC;
    std::vector<MemoryCell>::size_type currentGcThreshold;
    static const std::vector<MemoryCell>::size_type initialGcThreshold = 100;
    static const int gcGrowthFactor = 2;

    struct FrameGuard{
    private:
        LispRuntime *runtime;
        std::vector<LispRef>::size_type mark;

    public:
        FrameGuard(LispRuntime *runtime): runtime(runtime), mark(runtime->rootset.size()) { }
        ~FrameGuard() {
            runtime->rootset.resize(mark);
        }

        FrameGuard(const FrameGuard&) = delete;
        FrameGuard& operator=(const FrameGuard&) = delete;
    };

    const LispRef Nil_Ref = 0;
    const LispRef ParsingError_Ref = 1;
    const LispRef ArgumentCountMismatch_Ref = 2;
    const LispRef MemoryAccessOutOfBounds_Ref = 3;
    const LispRef MemoryUninitialized_Ref = 4;
    const LispRef MemoryOutOfSpace_Ref = 5;
    const LispRef True_Ref = 6;
    const LispRef False_Ref = 7;

    const LispRef GlobalEnvironment_Ref = Nil_Ref;

    void Mark() {
        // Start at the roots and mark all reachable memory cells
        std::vector<LispRef> stack(rootset);

        // Also include global symbols as part of the roots
        for (const SymbolTableEntry& entry : Symbols) {
            if (entry.Properties.ReBindable && entry.Properties.BoundReference != Nil_Ref) {
                stack.push_back(entry.Properties.BoundReference);
            }
        }

        while (!stack.empty()) {
            LispRef current = stack.back();
            stack.pop_back();

            if (current >= Heap.size())
                continue; // Out of bounds, ignore
            if ((Heap[current].Flags & MemoryFlags::Used) == 0)
                continue; // Current cell is not used, ignore
            if ((Heap[current].Flags & MemoryFlags::Marked) != 0)
                continue; // Already marked, ignore

            // Mark this cell
            Heap[current].Flags |= MemoryFlags::Marked;

            // If it's a cons cell, add its children to the stack
            const LispValue &val = Heap[current].Value;
            if (val.IsCons()) {
                stack.push_back(val.As.Cons.Car);
                stack.push_back(val.As.Cons.Cdr);
            }
            else if (val.IsClosure()) {
                // For closures, we also need to mark the body and environment
                stack.push_back(val.As.Closure.Body);
                stack.push_back(val.As.Closure.Environment);
            }
        }
    }

    void Sweep() {
        for (std::vector<MemoryCell>::size_type i = 0; i < Heap.size(); i++) {
            if ((Heap[i].Flags & MemoryFlags::Used) != 0 && (Heap[i].Flags & MemoryFlags::Marked) == 0) {
                Free(static_cast<LispRef>(i));
            } else {
                // Clear mark for next GC cycle
                Heap[i].Flags &= ~MemoryFlags::Marked;
            }
        }
        if (rootset.size() < (rootset.capacity() >> 1))
            rootset.shrink_to_fit();
    }

    void Collect() {
        Mark();
        Sweep();
    }

    bool TryAlloc(const LispValue &v, LispRef &reference, MemoryFlags additionalFlags = MemoryFlags::None) {
        // See if we need to trigger GC
        auto freeSpace = HeapFreeList.size();
        auto heapSize = Heap.size();
        unsigned int minGcThreshold = heapSize <= 20 ? 1 : heapSize / 20; // 5 % of the heap
        if (freeSpace < minGcThreshold || allocatedSinceLastGC >= currentGcThreshold) {
            // Always collect if 0 free space. Otherwise see if we meet some kind of "smart" GC condition TODO 
            Collect();
            allocatedSinceLastGC = 0;
            freeSpace = HeapFreeList.size();

            std::vector<MemoryCell>::size_type nextGcThreshold = (heapSize - freeSpace) * gcGrowthFactor;
            if (nextGcThreshold > heapSize)
                nextGcThreshold = heapSize;
            if (nextGcThreshold < initialGcThreshold)
                nextGcThreshold = initialGcThreshold;
            currentGcThreshold = nextGcThreshold;
        }

        // Allocate from free list
        if (freeSpace == 0)
            return false;

        std::vector<MemoryCell>::size_type free_index = HeapFreeList.back(); HeapFreeList.pop_back();
        MemoryCell &cell = Heap[free_index];
        
        cell.Flags |= additionalFlags | MemoryFlags::Used;
        cell.Value = v;
        reference = static_cast<LispRef>(free_index);
        allocatedSinceLastGC++;
        return true;
    }

    LispRef Alloc(const LispValue &v, MemoryFlags additionalFlags = MemoryFlags::None) {
        LispRef ref;
        if (TryAlloc(v, ref, additionalFlags)) {
            return ref;
        } else {
            return MemoryOutOfSpace_Ref;
        }
    }

    void Free(const LispRef reference) {
        if (reference >= Heap.size())
            return;
        if ((Heap[reference].Flags & MemoryFlags::Protected) != 0)
            return;

        Heap[reference].Flags &= ~(MemoryFlags::Used | MemoryFlags::Marked);
        Heap[reference].Value = LispValue::Error(LispErrorCode::MemoryUninitialized);
        HeapFreeList.push_back(static_cast<std::vector<MemoryCell>::size_type>(reference));
    }


    LispRef CreateBinding(SymbolRef symbolRef, LispRef valueRef, MemoryFlags flags = MemoryFlags::None) {
        LispValue bindingCons = LispValue::Cons(
            static_cast<LispRef>(symbolRef),    // Car: the symbol (seems wrong)
            valueRef                            // Cdr: the value
        );
        LispRef bindingRef;
        if (TryAlloc(bindingCons, bindingRef, flags)) {
            return bindingRef;
        } else {
            return MemoryOutOfSpace_Ref;
        }
    }

    LispRef ExtendEnvironment(LispRef parentEnv, SymbolRef symbolRef, LispRef valueRef) {
        LispRef binding = CreateBinding(symbolRef, valueRef);
        if (binding == MemoryOutOfSpace_Ref) return MemoryOutOfSpace_Ref;
        
        // New environment: (binding . parent-env)
        LispValue newEnv = LispValue::Cons(binding, parentEnv);
        LispRef envRef;
        if (!TryAlloc(newEnv, envRef, MemoryFlags::None)) {
            return MemoryOutOfSpace_Ref;
        }
        return envRef;
    }

    LispRef LookupSymbol(LispRef cellRef, SymbolRef symbolRef, LispRef environmentRef) {
        LispRef currentEnv = environmentRef;
        
        while (currentEnv != Nil_Ref) {
            const LispValue &envCell = ValueOf(currentEnv);
            if (!envCell.IsCons()) break;
            
            // Get the binding (head of the environment list)
            const LispValue &binding = ValueOf(envCell.As.Cons.Car);
            if (binding.IsCons()) {
                // binding.Car is actually the symbol (stored as a number, reinterpreted)
                // binding.Cdr is the value
                // This is a bit of a hack - we're storing SymbolRef in a LispRef slot
                if (binding.As.Cons.Car == static_cast<LispRef>(symbolRef)) {
                    return binding.As.Cons.Cdr;  // Found it!
                }
            }
            
            // Move to parent environment
            currentEnv = envCell.As.Cons.Cdr;
        }
        
        return cellRef;  // Not found in environment, just return the cell with the symbol
    }

public:
    LispRuntime(std::vector<MemoryCell>::size_type heap_size): Symbols(), Heap(heap_size), HeapFreeList(heap_size), rootset(), allocatedSinceLastGC(0), currentGcThreshold(initialGcThreshold) {
        // Generate the free list 
        for (unsigned int i = 0; i < heap_size; ++i) {
            HeapFreeList[i] = i;
        }

        // Pre-allocate some "special" values in the pool... maybe (that would allow Eval to always return a LispRef)
        Alloc(LispValue::Nil(), MemoryFlags::Protected);
        Alloc(LispValue::Error(LispErrorCode::ParsingError), MemoryFlags::Protected);
        Alloc(LispValue::Error(LispErrorCode::ArgumentCountMismatch), MemoryFlags::Protected);
        Alloc(LispValue::Error(LispErrorCode::MemoryAccessOutOfBounds), MemoryFlags::Protected);
        Alloc(LispValue::Error(LispErrorCode::MemoryUninitialized), MemoryFlags::Protected);
        Alloc(LispValue::Error(LispErrorCode::MemoryOutOfSpace), MemoryFlags::Protected);
        Alloc(LispValue::Number(1), MemoryFlags::Protected); // True
        Alloc(LispValue::Number(0), MemoryFlags::Protected); // 
    }

    // @brief Reset VM state, clear all allocated memory
    void Reset() {
        // Free all old memory
        for (std::vector<MemoryCell>::size_type i = 0; i < Heap.size(); i++) {
            Free(static_cast<LispRef>(i));
        }
        Symbols.Clear(); // Clear symbol table as well
        rootset.clear();
        allocatedSinceLastGC = 0;
        currentGcThreshold = initialGcThreshold;
    }

    // @brief Get a reference to the symbol table
    SymbolTable& GetSymbols() {
        return this->Symbols;
    }

    // @brief Get the actual value associated with the heap reference
    LispValue& ValueOf(LispRef root) {
        // Ensure in pool
        if (root >= Heap.size()) {
            return Heap[MemoryAccessOutOfBounds_Ref].Value;
        }
        
        // Ensure is a real value
        const MemoryCell &box = Heap[root];
        if ((box.Flags & MemoryFlags::Used) == 0) {
            return Heap[MemoryUninitialized_Ref].Value;
        }

        // Return a copy of the value
        return Heap[root].Value;
    }

    LispRef Eval(LispRef root, LispRef currentEnv) {
        // Ensure in pool
        if (root >= Heap.size()) {
            return MemoryAccessOutOfBounds_Ref;
        }
        
        // Ensure is a real value
        const MemoryCell &box = Heap[root];
        if ((box.Flags & MemoryFlags::Used) == 0) {
            return MemoryUninitialized_Ref;
        }

        FrameGuard guard(this); // Push a frameguard to auto remove any pushed back allocations from the rootset after this eval exits
        rootset.push_back(currentEnv);

        // Evaluate the reference to a value
        switch (box.Value.Type) {
            case LispValueType::Number:
            case LispValueType::Error:
            case LispValueType::Nil:
            case LispValueType::Closure:
                return root;
            
            case LispValueType::Symbol: 
                return LookupSymbol(root, box.Value.As.Symbol, currentEnv);
            case LispValueType::Cons: {
                    // (car, cdr) implies (func, args)
                    LispRef funcRef = Eval(box.Value.As.Cons.Car, currentEnv);
                    rootset.push_back(funcRef);
                    const LispValue &func = ValueOf(funcRef);

                    // Handle user defined closures
                    if (func.IsClosure()) {
                        // ((car, cdr), cdr) implies ((params, body), env) for format
                        const LispValue &closureBody = ValueOf(func.As.Closure.Body);
                        if (!closureBody.IsCons()) {
                            return MemoryUninitialized_Ref;  // Malformed closure
                        }

                        // Closure format is (params, body)
                        LispRef paramListRef = closureBody.As.Cons.Car;
                        LispRef bodyRef = closureBody.As.Cons.Cdr;
                        LispRef argListRef = box.Value.As.Cons.Cdr;

                        // Start with the captured environment
                        LispRef newEnv = func.As.Closure.Environment;

                        // Bind each parameter to it's argument in the new environment
                        LispRef currentParam = paramListRef;
                        LispRef currentArg = argListRef;

                        while (!ValueOf(currentParam).IsNil()) {
                            const LispValue &param = ValueOf(currentParam);
                            if (!param.IsCons())
                                break; // End of parameter list

                            SymbolRef paramSymbol = static_cast<SymbolRef>(param.As.Cons.Car); 

                            // Evaluate the corresponding argument
                            if (ValueOf(currentArg).IsNil()) {
                                return ArgumentCountMismatch_Ref;  // Argument count mismatch
                            }
                            const LispValue &arg = ValueOf(currentArg);
                            if (!arg.IsCons())
                                break;
                            
                            LispRef argValue = Eval(arg.As.Cons.Car, currentEnv);
                            rootset.push_back(argValue);

                            // Extend the environment with this binding
                            newEnv = ExtendEnvironment(newEnv, paramSymbol, argValue);
                            if (newEnv == MemoryOutOfSpace_Ref) {
                                return MemoryOutOfSpace_Ref;
                            }
                            rootset.push_back(newEnv);
                            
                            // Move to next parameter and argument
                            currentParam = param.As.Cons.Cdr;
                            currentArg = arg.As.Cons.Cdr;
                        }

                        return Eval(bodyRef, newEnv);
                    }

                    // Handle special built-ins
                    LispRef result = root; // Fallback to return the whole CONS when function cannot evaluate
                    TryExecBuiltin(currentEnv, func, box.Value.As.Cons.Cdr, result);
                    
                    // Otherwise, just return the list itself
                    return result;
                }
        }

        // Fallback case (should never be called)
        return Nil_Ref; 
    }

    LispRef Eval(LispRef root) {
        return Eval(root, GlobalEnvironment_Ref);
    }

    virtual bool TryExecBuiltin(const LispRef &currentEnv, const LispValue &func, const LispRef &argsRef, LispRef &result) {
        if (!func.IsSymbol())
            return false;
        
        const std::string &name = Symbols.GetName(func.As.Symbol);

        // By default, built-ins include
        
        // Complex builtins
        // define
        // lambda?
        if (name == "lambda") {
            // (lambda ((params) body))
            const LispValue &argsCell = ValueOf(argsRef);
            if (!argsCell.IsCons()) {
                // TODO return a better error here
                result = MemoryUninitialized_Ref;  // Malformed lambda 
                return true;
            }
            
            LispRef paramListRef = argsCell.As.Cons.Car;
            LispRef bodyRef = argsCell.As.Cons.Cdr;

            LispRef closureBodyRef;
            if (!TryAlloc(LispValue::Cons(paramListRef, bodyRef), closureBodyRef, MemoryFlags::None)) {
                result = MemoryOutOfSpace_Ref;
                return true;
            }

            LispRef closureRef;
            if (!TryAlloc(LispValue::Closure(closureBodyRef, currentEnv), closureRef, MemoryFlags::None)) {
                result = MemoryOutOfSpace_Ref;
                return true;
            }

            result = closureRef;
            return true;
        }
        else if (name == "define") {
            // Format of args is Cons(Symbol, ?)
            const LispValue &argsCell = ValueOf(argsRef);
            if (!argsCell.IsCons()) {
                // TODO return a better error here
                result = MemoryUninitialized_Ref;  // Malformed define 
                return true;
            }

            const LispValue &symbolCell = ValueOf(argsCell.As.Cons.Car);
            if (!symbolCell.IsSymbol()) {
                // TODO return a better error here
                result = MemoryUninitialized_Ref;  // Malformed define 
                return true;
            }

            // Evaluate the rest and bind the result
            const LispRef valueToBind = Eval(argsCell.As.Cons.Cdr, currentEnv);
            rootset.push_back(valueToBind);
            bool didBind = Symbols.Bind(symbolCell.As.Symbol, valueToBind);
            if (didBind) {
                result = valueToBind;
            } else {
                result = Nil_Ref; // If we didn't bind, return Nil
            }
            return true;
        }
        else if (name == "quote") {
            // Return the args as is, no processing
            result = argsRef;
            return true;
        }
        else if (name == "cond") {
            // Cond is a list of CONS where each CONS is (CAR(condition, body) ,CDR... )
            LispValue &argsCell = ValueOf(argsRef);
            if (!argsCell.IsCons()) {
                // TODO return a better error here
                result = MemoryUninitialized_Ref;  // Malformed cons 
                return true;
            }

            result = Nil_Ref;
            LispRef listCellRef = argsRef;
            while (!ValueOf(listCellRef).IsNil()) {
                const LispValue &listCell = ValueOf(listCellRef);
                const LispValue &condition_body_pair = ValueOf(listCell.As.Cons.Car);
                if (!condition_body_pair.IsCons()) {
                    listCellRef = listCell.As.Cons.Cdr;
                    continue;
                }
                
                LispRef condition = condition_body_pair.As.Cons.Car;
                LispRef body = condition_body_pair.As.Cons.Cdr;
                
                LispRef evaledCondition = Eval(condition, currentEnv);
                rootset.push_back(evaledCondition);
                if (ValueOf(evaledCondition).AsBoolean()) {
                    result = Eval(body, currentEnv);
                    rootset.push_back(result);
                    break;
                } else {
                    listCellRef = listCell.As.Cons.Cdr;
                    continue;
                }
            }
            return true;
        }

        // Simple built-ins
        // +
        // -
        // * 
        // /
        // car
        // cdr
        // cons
        std::vector<LispRef> evaluatedArgs;
        LispRef currentArg = argsRef;
        while (!ValueOf(currentArg).IsNil()) {
            const LispValue &argCell = ValueOf(currentArg);
            if (!argCell.IsCons()) break;
            
            LispRef evaledArg = Eval(argCell.As.Cons.Car, currentEnv);
            evaluatedArgs.push_back(evaledArg);
            rootset.push_back(evaledArg);
            
            currentArg = argCell.As.Cons.Cdr;
        }

        if (name == "+") {
            LispValue sum = LispValue::Number(0);
            for (LispRef argRef: evaluatedArgs) {
                sum = sum.Add(ValueOf(argRef));
            }
            result = MemoryOutOfSpace_Ref;
            TryAlloc(sum, result, MemoryFlags::None);
            return true;
        } 
        else if (name == "-") {
            if (evaluatedArgs.size() < 1) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            LispValue difference = ValueOf(evaluatedArgs[0]);
            for (size_t i = 1; i < evaluatedArgs.size(); i++) {
                difference = difference.Sub(ValueOf(evaluatedArgs[i]));
            }

            result = MemoryOutOfSpace_Ref;
            TryAlloc(difference, result, MemoryFlags::None);
            return true;
        } 
        else if (name == "*") {
            LispValue product = LispValue::Number(1);
            for (LispRef argRef: evaluatedArgs) {
                product = product.Mul(ValueOf(argRef));
            }
            result = MemoryOutOfSpace_Ref;
            TryAlloc(product, result, MemoryFlags::None);
            return true;
        }
        else if (name == "/") {
            // Min 2, if more than 2, repeated division (/ 4 5 6) => (/ (/ 4 5) 6)
            if (evaluatedArgs.size() < 2) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            LispValue quotient = ValueOf(evaluatedArgs[0]);
            for (size_t i = 1; i < evaluatedArgs.size(); i++) {
                quotient = quotient.Div(ValueOf(evaluatedArgs[i]));
            }

            result = MemoryOutOfSpace_Ref;
            TryAlloc(quotient, result, MemoryFlags::None);
            return true;
        }
        else if (name == ">") {
            if (evaluatedArgs.size() != 2) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            result = ValueOf(evaluatedArgs[0]).Gt(ValueOf(evaluatedArgs[1])) ? True_Ref : False_Ref;
            return true;
        }
        else if (name == "<") {
            if (evaluatedArgs.size() != 2) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            result = ValueOf(evaluatedArgs[0]).Lt(ValueOf(evaluatedArgs[1])) ? True_Ref : False_Ref;
            return true;
        }
        else if (name == "=") {
            if (evaluatedArgs.size() < 2) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            bool eq = true;
            const LispValue &first = ValueOf(evaluatedArgs[0]);
            for (size_t i = 1; i < evaluatedArgs.size(); i++) {
                eq &= first.Eq(ValueOf(evaluatedArgs[i]));
                if (!eq)
                    break; // Stop as soon as we fail (fail fast)
            }
            result = eq ? True_Ref : False_Ref;
            return true;
        }
        else if (name == "car") {
            if (evaluatedArgs.size() != 1) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            const LispValue &consVal = ValueOf(evaluatedArgs[0]);
            if (!consVal.IsCons()) {
                result = Nil_Ref;
                return true;
            }
            result = consVal.As.Cons.Car;  // Return reference to car
            return true;
        }
        else if (name == "cdr") {
            if (evaluatedArgs.size() != 1) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            const LispValue &consVal = ValueOf(evaluatedArgs[0]);
            if (!consVal.IsCons()) {
                result = Nil_Ref;
                return true;
            }
            result = consVal.As.Cons.Cdr;  // Return reference to cdr
            return true;
        }
        else if (name == "cons") {
            if (evaluatedArgs.size() != 2) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            LispRef newConsRef = MemoryOutOfSpace_Ref;
            TryAlloc(LispValue::Cons(evaluatedArgs[0], evaluatedArgs[1]), newConsRef, MemoryFlags::None);
            
            result = newConsRef;
            return true;
        }
        else if (name == "atom?") {
            if (evaluatedArgs.size() != 1) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            const LispValue &val = ValueOf(evaluatedArgs[0]);
            if (val.IsAtom()) {
                result = True_Ref;
            } else {
                result = False_Ref;
            }
            return true;
        }
        else if (name == "list?") {
            if (evaluatedArgs.size() != 1) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            const LispValue &val = ValueOf(evaluatedArgs[0]);
            if (val.IsList()) {
                result = True_Ref;
            } else {
                result = False_Ref;
            }
            return true;
        }
        else if (name == "and") {
            bool res = true;
            for (size_t i = 0; i < evaluatedArgs.size(); i++) {
                res &= ValueOf(evaluatedArgs[i]).AsBoolean();
            }  

            result = res == true ? True_Ref : False_Ref ;
            return true;
        } 
        else if (name == "or") {
            bool res = false;
            for (size_t i = 0; i < evaluatedArgs.size(); i++) {
                res |= ValueOf(evaluatedArgs[i]).AsBoolean();
            }  

            result = res == true ? True_Ref : False_Ref ;
            return true;
        } 
        else if (name == "xor") {
            if (evaluatedArgs.size() != 2) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            bool res = ValueOf(evaluatedArgs[0]).AsBoolean() ^ ValueOf(evaluatedArgs[1]).AsBoolean();

            result = res == true ? True_Ref : False_Ref ;
            return true;
        }
        else if (name == "not") {
            if (evaluatedArgs.size() != 1) {
                result = ArgumentCountMismatch_Ref;
                return true;
            }

            bool val = ValueOf(evaluatedArgs[0]).AsBoolean();

            result = val == true ? False_Ref : True_Ref ;
            return true;
        }

        // Was not a builtin
        return false;
    }

    bool DidParseSuccessfully() {
        return this->last_parse_error == ParseErrorCode::None;
    }

    ParseErrorCode GetParsingError() {
        return this->last_parse_error;
    }

private:
#pragma region Parsing
    inline bool is_whitespace(char c) {
        return c == ' ' || c == '\n' || c == '\r' || c == '\b' || c == '\t' || c == '\f' || c == '\v';
    }
    inline bool is_digit(char c) {
        return c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9';
    }
    inline bool is_sign(char c) {
        return c == '+' || c == '-';
    }
    inline bool is_operator(char c) {
        return c == '+' || c == '-' || c == '*' || c == '/' || c == '?' || c == '>' || c == '<' || c == '=';
    }
    inline bool is_sym_start(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || is_operator(c);
    }
    inline bool is_sym_rest(char c) {
        return is_sym_start(c) || is_digit(c);
    }

    ParseErrorCode last_parse_error = ParseErrorCode::None;
    LispRef parse_error(ParseErrorCode code = ParseErrorCode::Generic) {
        this->last_parse_error = code;
        return ParsingError_Ref;
    }

    void skip_whitespace(ICharStream &stream) {
        while (!stream.Eof()) {
            int c = stream.Peek();
            // Normal whitespace
            if (is_whitespace(c)) {
                stream.Advance();
                continue;
            }
            // Comments
            else if (c == ';') {
                while (!stream.Eof() && c != '\n') 
                    c = stream.Advance();
                continue;
            }
            // Not whitespace
            else {
                break;
            }
        }
    }

    // <signed_integer> ::= (- | +)? [0-9]+
    LispRef parse_signed_integer(ICharStream &stream) {
        int sign = 1;
        if (stream.Peek() == '-') {
            if (!is_digit(stream.PeekFuture()))
                return parse_error(ParseErrorCode::InvalidToken);
            sign = -1;
            stream.Advance();
        }
        else if (stream.Peek() == '+') {
            if (!is_digit(stream.PeekFuture()))
                return parse_error(ParseErrorCode::InvalidToken);
            stream.Advance();
        }

        int value = 0;
        while (is_digit(stream.Peek())) {
            value = value * 10 + (stream.Advance() - '0');
        }

        return Alloc(LispValue::Number(sign * value));
    }

    // <number> ::= <signed_integer>
    LispRef parse_number(ICharStream &stream) {
        // TODO allow other number types
        return parse_signed_integer(stream);
    }

    // <symbol> ::= <c-identifier> | <operator>
    const int MAX_SYMBOL_LEN = 32;
    LispRef parse_symbol(ICharStream &stream) {
        char buf[MAX_SYMBOL_LEN];
        int len = 0;

        // Read up to a max of MAX_SYMBOL_LEN characters
        while (len < MAX_SYMBOL_LEN && is_sym_rest(stream.Peek())) {
            buf[len++] = stream.Advance();
        }

        // Ignore remaining characters (if there are more than MAX_SYMBOL_LEN just skip as they aren't part of the symbol)
        // I want this because its similar to old C
        while (is_sym_rest(stream.Peek())) {
            stream.Advance();
        }

        SymbolRef sym = Symbols.Intern(std::string(buf, len));
        return Alloc(LispValue::Symbol(sym));
    }

    // <list> ::= '(' <expr>* ')'
    LispRef parse_list(ICharStream &stream) {
        if (stream.Eof())
            return parse_error(ParseErrorCode::EndOfFile);

        // Skip '('
        stream.Advance();
        
        LispRef head = Nil_Ref; // Nil_Ref is a LispRef to the constant NIL stored in the heap
        LispRef tail = Nil_Ref; // Invalid until the first element

        while (true) {
            skip_whitespace(stream);
            
            // Error end of file
            if (stream.Eof()) {
                return parse_error(ParseErrorCode::EndOfFile);
            }

            // Stop
            if (stream.Peek() == ')') {
                stream.Advance();
                return head;
            }   

            // Parse the expression
            LispRef elem = parse_expr(stream);
            if (ValueOf(elem).IsError())
                return elem;

            // Allocate cell 
            LispRef cell = Alloc(LispValue::Cons(elem, Nil_Ref));
            if (ValueOf(cell).IsError())
                return cell;

            if (ValueOf(head).IsNil()) {
                // First element
                head = cell;
                tail = cell;
            } else {
                // Append to tail
                ValueOf(tail).As.Cons.Cdr = cell;
                tail = cell;
            }
        }

        return parse_error(ParseErrorCode::EndOfFile);
    }

    // <expr> ::= <number> | <symbol> | <list>
    LispRef parse_expr(ICharStream &stream) {
        skip_whitespace(stream);

        char c = stream.Peek();

        if (c == '(') return parse_list(stream);
        if (c == ')') return parse_error(ParseErrorCode::InvalidToken);
        if (is_digit(c) || ((c == '-' || c == '+') && is_digit(stream.PeekFuture()))) return parse_number(stream);
        if (is_sym_start(c)) return parse_symbol(stream);

        return parse_error(ParseErrorCode::InvalidToken);
    }

    // <expr-list> ::= <expr>*
    std::vector<LispRef> parse_expr_list(ICharStream &stream) {
        std::vector<LispRef> exprs;

        while (!stream.Eof()) {
            skip_whitespace(stream);
            if (stream.Eof()) break;

            LispRef expr = parse_expr(stream);
            exprs.push_back(expr);

            if (ValueOf(expr).IsError())
                return exprs; // Return the error immediately if any expression fails to parse
        }

        return exprs;
    }

public:
    std::vector<LispRef> Parse(ICharStream &stream) {
        this->last_parse_error = ParseErrorCode::None; // Reset parse error state
        return parse_expr_list(stream);
    }
    
    LispRef EvalAll(std::vector<LispRef> &exprs) {
        LispRef lastResult = Nil_Ref;
        for (LispRef expr: exprs) {
            lastResult = Eval(expr);
        }
        return lastResult;
    }
#pragma endregion

    std::string Stringify(LispRef ref) {
        const LispValue &val = ValueOf(ref);
        switch (val.Type) {
            case LispValueType::Nil:
                return "nil";
            case LispValueType::Number:
                return std::to_string(val.As.Number);
            case LispValueType::Symbol:
                return Symbols.GetName(val.As.Symbol);
            case LispValueType::Cons:
                return "(" + Stringify(val.As.Cons.Car) + " " + Stringify(val.As.Cons.Cdr) + ")";
            case LispValueType::Closure:
                return "<closure>";
            case LispValueType::Error:
                return "<error: " + Enum2String(val.ErrorType) + ">";
            default:
                return "<unknown>";
        }
    }
};

// Usage 
// LispRuntime runtime(1024); // Heap size of 1024 cells
// ICharStream &stream = ...; // Implement an ICharStream for your input source
// std::vector<LispRef> exprs = runtime.Parse(stream);
// if (!runtime.DidParseSuccessfully()) {
//     std::cerr << "Parse error: " << static_cast<uint8_t>(runtime.GetParsingError()) << std::endl;
//     return 1;
// }
// LispRef result = runtime.EvalAll(exprs);
// cout << runtime.Stringify(result) << endl;
// return 0;

// If reusing the runtime, you can reset it to clear memory and symbol table
// runtime.Reset(); 

#endif