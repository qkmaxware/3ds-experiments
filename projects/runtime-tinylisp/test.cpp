#include "lisp.hpp"
#include <cstdio>

class StringStream: public ICharStream {
private:
    std::string src;
    int ptr;

public:
    StringStream(const std::string &str): src(str), ptr(0) { }

    bool Eof() const override {
        return ptr >= src.size();
    }
    char Peek() const override {
        if (Eof())
            return '\0';
        return src[ptr];
    }
    char PeekFuture() const override {
        if ((ptr + 1) < src.size())
            return src[ptr + 1];
        return '\0';
    }
    char Advance() override {
        char c = Peek();
        if (!Eof())
            ptr += 1;
        return c;
    }
};

class Runtime {
private:
    std::string script_path;
    LispRuntime runtime;

public:
    Runtime(): script_path(), runtime(10000) {}

    void setup() {

    }

    void cleanup() {

    }
};

int main() {
    LispRuntime runtime(1024);

    runtime.Reset();
    StringStream stream("(+ 4 5 6)"); 
    std::vector<LispRef> exprs = runtime.Parse(stream); // will be parsed as (+ (4 (5 (6 nil))))
    if (!runtime.DidParseSuccessfully()) {
        std::printf("Failed to parse\n");
        return 1;
    }

    std::printf("number of statements: %i\n", exprs.size());
    int i = 1;
    for (LispRef reference : exprs) {
        LispValue& val = runtime.ValueOf(reference);
        std::string str = runtime.Stringify(reference);
        std::printf("  %i: (%s) %s\n", i, LispValueTypeString(val.Type).c_str(), str.c_str());
        i++;
    }
    LispRef result = runtime.EvalAll(exprs); // Hopefully will be the value 15, at the moment it' not, it's nil
    std::string fmt = runtime.Stringify(result);
    std::printf("result is: %s\n", fmt.c_str());

    return 0;
}